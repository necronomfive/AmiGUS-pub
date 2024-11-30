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

#include "amigus_hardware.h"
#include "amigus_private.h"
#include "debug.h"
#include "SDI_AHI4_protos.h"

/* Basic functions - Actions */

ASM(void) SAVEDS AHIsub_Disable(
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


ASM(void) SAVEDS AHIsub_Enable(
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

    LOG_D(("D: Creating playback buffers\n", (LONG) AmiGUSBase));
    if ( CreatePlaybackBuffers() ) {

      LOG_D(("D: No playback buffers, failed.\n"));
      return AHIE_UNKNOWN;
    }
  }

 if ( AHISF_RECORD & aFlags ) {

    DisplayError( ERecordingNotImplemented );
    return AHIE_UNKNOWN;
  }

  LOG_D(("D: Creating worker process for AmiGUSBase @ %08lx\n", (LONG) AmiGUSBase));
  if ( CreateWorkerProcess() ) {

    LOG_D(("D: No worker, failed.\n"));
    return AHIE_UNKNOWN;
  }
  if ( CreateInterruptHandler() ) {
  
    LOG_D(("D: No INT handler, failed.\n"));
    return AHIE_UNKNOWN;
  }

  initAmiGUS();
  LOG_D(("D: AHIsub_Start done\n"));
  return AHIE_OK;
}

ASM(void) SAVEDS AMIGA_INTERRUPT AHIsub_Update(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(("D: AHIsub_Update start\n"));
  LOG_V(("V: old ctrl 0x%08lx, new ctrl 0x%08lx, longSize %ld, BuffSamples %lu, Min %lu, Max %lu, BuffSize %lu, BuffType %lu\n",
          AmiGUSBase->agb_AudioCtrl,
          aAudioCtrl,
          AmiGUSBase->agb_BufferSize,
          aAudioCtrl->ahiac_BuffSamples,
          aAudioCtrl->ahiac_MinBuffSamples,
          aAudioCtrl->ahiac_MaxBuffSamples,
          aAudioCtrl->ahiac_BuffSize,
          aAudioCtrl->ahiac_BuffType
       ));
  AmiGUSBase->agb_AudioCtrl = aAudioCtrl;
  LOG_D(("D: AHIsub_Update stop\n"));
  return;
}

ASM(void) SAVEDS AHIsub_Stop(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(("D: AHIsub_Stop start\n"));

  LOG_D(("D: Read FIFO level %04lx\n",
         ReadReg16(
             AmiGUSBase->agb_CardBase,
             AMIGUS_MAIN_FIFO_USAGE) ));
  stopAmiGUS();

  DestroyInterruptHandler();
  DestroyWorkerProcess();
  DestroyPlaybackBuffers();


  LOG_D(("AHIsub_Stop done\n"));
  return;
}
