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

#include "amigus_pcm.h"
#include "amigus_private.h"
#include "debug.h"
#include "errors.h"
#include "interrupt.h"
#include "support.h"
#include "SDI_AHI4_protos.h"
#include "worker.h"

/* Basic functions - Actions */

ASM(VOID) SAVEDS AHIsub_Disable(
  REG(a6, struct Library* aBase),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
/*
  LOG_D(("AHIsub_Disable\n"));
  WriteReg16( AmiGUSBase->agb_CardBase,
              AMIGUS_MAIN_INT_ENABLE,
              AMIGUS_INT_FLAG_MASK_CLEAR
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK
            );
*/
  Disable();
  return;
}


ASM(VOID) SAVEDS AHIsub_Enable(
  REG(a6, struct Library* aBase),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
/*
  LOG_D(("AHIsub_Enable\n"));
  WriteReg16( AmiGUSBase->agb_CardBase,
              AMIGUS_MAIN_INT_ENABLE,
              AMIGUS_INT_FLAG_MASK_SET
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK
            );
         */
  Enable();
  return;
}

ASM(ULONG) SAVEDS AHIsub_Start(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(("D: AHIsub_Start start\n"));

  AHIsub_Update( aBase, aFlags, aAudioCtrl );

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

  StartAmiGusPcmPlayback();
  LOG_D(( "D: AHIsub_Start done\n" ));
  return AHIE_OK;
}

ASM(VOID) SAVEDS AMIGA_INTERRUPT AHIsub_Update(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {

  ULONG alignedSamples;
  UBYTE sampleToByte = AmiGUSBase->agb_AhiSampleShift;
  UWORD hwSampleSize = AmiGUSSampleSizes[ AmiGUSBase->agb_HwSampleFormat ];
  ULONG alignedSamplesHwWordSize;

  LOG_D(( "D: AHIsub_Update start\n" ));
  if ( AmiGUSBase->agb_AudioCtrl ) {
    LOG_V(( "V: Old ctrl 0x%08lx - "
            "Size %lu Samples %lu Min %lu Max %lu Type %lu\n",
            AmiGUSBase->agb_AudioCtrl,
            AmiGUSBase->agb_AudioCtrl->ahiac_BuffSize,
            AmiGUSBase->agb_AudioCtrl->ahiac_BuffSamples,
            AmiGUSBase->agb_AudioCtrl->ahiac_MinBuffSamples,
            AmiGUSBase->agb_AudioCtrl->ahiac_MaxBuffSamples,
            AmiGUSBase->agb_AudioCtrl->ahiac_BuffType
        ));
  }
  LOG_V(( "V: New ctrl 0x%08lx - "
          "Size %lu Samples %lu Min %lu Max %lu Type %lu\n",
          aAudioCtrl,
          aAudioCtrl->ahiac_BuffSize,
          aAudioCtrl->ahiac_BuffSamples,
          aAudioCtrl->ahiac_MinBuffSamples,
          aAudioCtrl->ahiac_MaxBuffSamples,
          aAudioCtrl->ahiac_BuffType
       ));
  alignedSamples = 
    AlignByteSizeForSamples( aAudioCtrl->ahiac_BuffSamples ) >> sampleToByte;
  aAudioCtrl->ahiac_BuffSamples = alignedSamples;
  AmiGUSBase->agb_AudioCtrl = aAudioCtrl;

  /* Finally, adapt watermark, ticking in hardware samples in WORDs! */
  
  alignedSamplesHwWordSize = UMult32( alignedSamples, hwSampleSize ) >> 1;
  if ( ( AMIGUS_PCM_PLAY_FIFO_WORDS >> 1 ) < alignedSamplesHwWordSize ) {

    AmiGUSBase->agb_watermark = AMIGUS_PCM_PLAY_FIFO_WORDS >> 1;

  } else {

    AmiGUSBase->agb_watermark = alignedSamplesHwWordSize;
  }
  LOG_D(( "D: Mix / copy up to %ld WORDs from AHI per pass, "
          "converts to %ld WORDs in AmiGUS, using watermark %ld WORDs\n",
          ( alignedSamples << sampleToByte ) >> 1,
          alignedSamplesHwWordSize,
          AmiGUSBase->agb_watermark ));

  LOG_D(( "D: AHIsub_Update done.\n" ));
  return;
}

ASM(VOID) SAVEDS AHIsub_Stop(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(( "D: AHIsub_Stop start\n" ));

  LOG_D(( "D: Read FIFO level %04lx\n",
          ReadReg16(
            AmiGUSBase->agb_CardBase,
            AMIGUS_PCM_PLAY_FIFO_USAGE ) ));
  StopAmiGusPcmPlayback();

  DestroyInterruptHandler();
  DestroyWorkerProcess();
  DestroyPlaybackBuffers();

  LOG_D(( "D: AHIsub_Stop done\n" ));

  return;
}
