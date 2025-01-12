/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/utility.h>

#include "amigus_ahi_sub.h"
#include "amigus_pcm.h"
#include "amigus_hardware.h"
#include "buffers.h"
#include "debug.h"
#include "errors.h"
#include "interrupt.h"
#include "support.h"
#include "SDI_ahi_sub.h"
#include "worker.h"

/* Basic functions - Actions */

ASM(VOID) SAVEDS AHIsub_Disable(
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {

  // LOG_D(("AHIsub_Disable\n"));
#if 0
  WriteReg16( AmiGUSBase->agb_CardBase,
              AMIGUS_PCM_MAIN_INT_ENABLE,
              AMIGUS_INT_F_MASK_CLEAR
            | AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK
            | AMIGUS_INT_F_REC_FIFO_FULL
            | AMIGUS_INT_F_REC_FIFO_WATERMARK );
#else
  Disable();
#endif
  return;
}


ASM(VOID) SAVEDS AHIsub_Enable(
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {

#if 0
  UWORD targetState = AMIGUS_INT_F_MASK_SET;

  // LOG_D(("AHIsub_Enable\n"));
  if ( AMIGUS_AHI_F_PLAY_STARTED & AmiGUSBase->agb_StateFlags ) {

    targetState |=
      AMIGUS_INT_F_PLAY_FIFO_EMPTY | AMIGUS_INT_F_PLAY_FIFO_WATERMARK;
  }
  if ( AMIGUS_AHI_F_REC_STARTED & AmiGUSBase->agb_StateFlags ) {

    targetState |=
      AMIGUS_INT_F_REC_FIFO_FULL | AMIGUS_INT_F_REC_FIFO_WATERMARK;
  }
  WriteReg16(
    AmiGUSBase->agb_CardBase,
    AMIGUS_PCM_MAIN_INT_ENABLE,
    targetState );
#else
  Enable();
#endif
  return;
}

ASM(ULONG) SAVEDS AHIsub_Start(
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(("D: AHIsub_Start start\n"));

  AHIsub_Update( aFlags, aAudioCtrl );

  if ( AHISF_PLAY & aFlags ) {

    LOG_D(("D: Creating playback buffers\n" ));
    if ( CreatePlaybackBuffers() ) {

      LOG_D(("D: No playback buffers, failed.\n"));
      DisplayError( EAllocatePlaybackBuffers );
      return AHIE_UNKNOWN;
    }
  }

 if ( AHISF_RECORD & aFlags ) {

    if ( !AmiGUSBase->agb_CanRecord ) {
      
      DisplayError( ERecordingModeNotSupported );
      return AHIE_UNKNOWN;
    }
    LOG_D(("D: Creating recording buffers\n" ));
    if ( CreateRecordingBuffers() ) {

      LOG_D(("D: No recording buffers, failed.\n"));
      DisplayError( EAllocateRecordingBuffers );
      return AHIE_UNKNOWN;
    }
  }

  LOG_D(( "D: Creating worker process for AmiGUSBase @ %08lx\n",
          (LONG) AmiGUSBase ));
  if ( CreateWorkerProcess() ) {

    LOG_D(( "D: No worker, failed.\n" ));
    return AHIE_UNKNOWN;
  }
  if ( CreateInterruptHandler() ) {
  
    LOG_D(( "D: No INT handler, failed.\n" ));
    return AHIE_UNKNOWN;
  }

  if ( AHISF_PLAY & aFlags ) {

    LOG_D(("D: Starting playback\n" ));
    StartAmiGusPcmPlayback();
  }
  if ( AHISF_RECORD & aFlags ) {

    LOG_D(("D: Starting recording\n" ));
    StartAmiGusPcmRecording();
  }
  LOG_D(( "D: AHIsub_Start done\n" ));
  return AHIE_OK;
}

ASM(VOID) SAVEDS AHIsub_Update(
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *newAudioCtrl)
) {

  const struct AHIAudioCtrlDrv *oldAudioCtrl = AmiGUSBase->agb_AudioCtrl;

  LOG_D(( "D: AHIsub_Update start\n" ));
  if ( oldAudioCtrl ) {

    LOG_V(( "V: Old ctrl 0x%08lx - "
            "Size %lu Samples %lu Min %lu Max %lu Type %lu\n",
            oldAudioCtrl,
            oldAudioCtrl->ahiac_BuffSize,
            oldAudioCtrl->ahiac_BuffSamples,
            oldAudioCtrl->ahiac_MinBuffSamples,
            oldAudioCtrl->ahiac_MaxBuffSamples,
            oldAudioCtrl->ahiac_BuffType ));
  } else {

    LOG_V(( "V: Old ctrl 0x%08lx\n", oldAudioCtrl ));
  }
  LOG_V(( "V: New ctrl 0x%08lx - "
          "Size %lu Samples %lu Min %lu Max %lu Type %lu\n",
          newAudioCtrl,
          newAudioCtrl->ahiac_BuffSize,
          newAudioCtrl->ahiac_BuffSamples,
          newAudioCtrl->ahiac_MinBuffSamples,
          newAudioCtrl->ahiac_MaxBuffSamples,
          newAudioCtrl->ahiac_BuffType ));
  AmiGUSBase->agb_AudioCtrl = newAudioCtrl;

  if ( AHISF_PLAY & aFlags ) {

    struct AmiGUSPcmPlayback *playback = &AmiGUSBase->agb_Playback;
    UBYTE sampleToByte = 
      AmiGUSBase->agb_AhiSampleShift;
    UWORD hwSampleSize = 
      AmiGUSPlaybackSampleSizes[ playback->agpp_HwSampleFormatId ];
    ULONG alignedSamples = 
      AlignByteSizeForSamples( newAudioCtrl->ahiac_BuffSamples )
        >> sampleToByte;
    ULONG alignedSamplesHwWordSize =
      UMult32( alignedSamples, hwSampleSize ) >> 1;

    newAudioCtrl->ahiac_BuffSamples = alignedSamples;
    /* Finally, adapt watermark, ticking in hardware samples in WORDs! */
    if ( ( AMIGUS_PCM_PLAY_FIFO_WORDS >> 1 ) < alignedSamplesHwWordSize ) {

      playback->agpp_Watermark = AMIGUS_PCM_PLAY_FIFO_WORDS >> 1;

    } else {

      playback->agpp_Watermark = alignedSamplesHwWordSize;
    }

    LOG_D(( "D: Mix / copy up to %ld WORDs from AHI per pass, "
            "converts to %ld WORDs in AmiGUS, using watermark %ld WORDs\n",
            ( alignedSamples << sampleToByte ) >> 1,
            alignedSamplesHwWordSize,
            playback->agpp_Watermark ));
  }

  LOG_D(( "D: AHIsub_Update done.\n" ));
  return;
}

ASM(VOID) SAVEDS AHIsub_Stop(
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(( "D: AHIsub_Stop start\n" ));

  if ( AHISF_PLAY & aFlags ) {

    LOG_D(( "D: Read final playback FIFO level %04lx, stopping now.\n",
            ReadReg16(
              AmiGUSBase->agb_CardBase,
              AMIGUS_PCM_PLAY_FIFO_USAGE ) ));

    StopAmiGusPcmPlayback();
    DestroyPlaybackBuffers();
  }
  if ( AHISF_RECORD & aFlags ) {

    LOG_D(( "D: Read final recording FIFO level %04lx, stopping now.\n",
            ReadReg16(
              AmiGUSBase->agb_CardBase,
              AMIGUS_PCM_REC_FIFO_USAGE ) ));
    StopAmiGusPcmRecording();
    DestroyRecordingBuffers();
  }
  if (!(( AMIGUS_AHI_F_PLAY_STARTED
        | AMIGUS_AHI_F_REC_STARTED ) & AmiGUSBase->agb_StateFlags )) {

    LOG_D(( "D: No playback or recording, "
            "ending interrupt handler and worker task.\n" ));
    DestroyInterruptHandler();
    DestroyWorkerProcess();

  } else {

    LOG_D(( "D: Driver still in used state, 0x%lx\n",
            AmiGUSBase->agb_StateFlags ));
  }

  LOG_D(( "D: AHIsub_Stop done\n" ));
  return;
}
