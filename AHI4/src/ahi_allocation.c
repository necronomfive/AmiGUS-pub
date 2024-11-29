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

#include "amigus_private.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "SDI_AHI4_protos.h"

/* Basic functions - Allocation */

ASM(ULONG) SAVEDS AHIsub_AllocAudio(
//  REG(a6, struct Library* aBase) works fine with vbcc, but not with SASC,
  REG(a1, struct TagItem* aTagList),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
  struct TagItem *stateTag = 0;
  struct TagItem *tag = 0;
  ULONG frequencyId = 0;
  ULONG frequency = 0;
  ULONG samplesAmiGus = 0;
  ULONG samplesAhi = 0;
  ULONG reminder = 0;
  UBYTE isStereo = FALSE;
  UBYTE isHifi = FALSE;
  UBYTE isRealtime = FALSE;
  UBYTE bitsPerAmiGusSample = 0;

  /* Will rely on AHI provided mixing and timing */
  ULONG result = AHISF_MIXING | AHISF_TIMING;

  LOG_D(("D: AHIsub_AllocAudio start\n"));
  LogTicks(0x03);

  /*
   * ------------------------------------------------------
   * Part 1: Allocate this AmiGUS to exactly this callee,
   *         re-entrance/interrupt/multitasking safe.
   * ------------------------------------------------------
   */
  Disable();
  if ( AmiGUSBase->agb_UsageCounter ) {

    Enable();
    DisplayError( EDriverInUse );
    return AHISF_ERROR;
  
  } else {

    ++AmiGUSBase->agb_UsageCounter;
    Enable();
  }
  LOG_D(("D: Alloc`ed AmiGUS AHI hardware\n"));

  /*
   * ------------------------------------------------------
   * Part 2: Extract audio mode information
   * ------------------------------------------------------
   */
  /* Find nearest supported frequency and provide it back */
  frequencyId = FindNearestFrequencyIndex( aAudioCtrl->ahiac_MixFreq );
  frequency = AmiGUSSampleRates[ frequencyId ];
  LOG_V(("D: Using %ldHz = 0x%02lx for requested %ldHz\n",
         frequency,
         frequencyId,
         aAudioCtrl->ahiac_MixFreq));
  aAudioCtrl->ahiac_MixFreq = frequency;

  /* Parse aTagList */
  stateTag = aTagList;
  while ( tag = NextTagItem( &stateTag ) ) {

    switch ( tag->ti_Tag ) {
      case AHIDB_Bits: {
        bitsPerAmiGusSample = (UBYTE)tag->ti_Data;
        break;
      }
      case AHIDB_Stereo: {
        isStereo = (UBYTE)tag->ti_Data;
        result |= AHISF_KNOWSTEREO;
        break;
      }
      case AHIDB_HiFi: {
        isHifi = (UBYTE)tag->ti_Data;
        LOG_D(("D: TODO: Why is HiFi in the list?\n"));
//        result |= AHISF_KNOWHIFI;
        break;
      }
      case AHIDB_Realtime: {
        isRealtime = (UBYTE)tag->ti_Data;
        break;
      }
      default: {
        break;
      }
    }
  }

  LOG_D(("D: Mode is %ldbit, %ld stereo, %ld HiFi, %ld Realtime, %ldHz\n",
         bitsPerAmiGusSample, isStereo, isHifi, isRealtime, frequency
       ));
  if (( isHifi ) || ( !isStereo ) || ( 16 != bitsPerAmiGusSample )) {
    DisplayError( EAudioModeNotImplemented );
    return AHISF_ERROR;
  }

  /*
   * ------------------------------------------------------
   * Part 3: Apply information to AmiGUS & driver.
   * ------------------------------------------------------
   */

  // TODO: Initialize AmiGUS with that information.

  // TODO: switch copy functions here for HiFi modes
  //       Others will be plain LONG copys.

  /*
   * ------------------------------------------------------
   * Part 4: Prepare slave task communication.
   * ------------------------------------------------------
   */
  AmiGUSBase->agb_WorkerWorkSignal = -1;
  AmiGUSBase->agb_WorkerStopSignal = -1;
  AmiGUSBase->agb_MainProcess = ( struct Process * ) FindTask( NULL );
  AmiGUSBase->agb_MainSignal = AllocSignal( -1 );
  if ( -1 == AmiGUSBase->agb_MainSignal ) {

    DisplayError( EMainProcessSignalsFailed );
    return AHISF_ERROR;
  }
  AmiGUSBase->agb_WorkerReady = FALSE;

  LOG_D(("D: AHIsub_AllocAudio done, expected %ld actual %ld\n", 
         AHISF_MIXING | AHISF_TIMING | AHISF_KNOWSTEREO,
         result));

  return result;
}

ASM(void) SAVEDS AHIsub_FreeAudio(
  REG(a6, struct Library* aBase),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(("D: AHIsub_FreeAudio start\n"));

  /*
   * ------------------------------------------------------
   * Part 4: Free slave task communication.
   * ------------------------------------------------------
   */
  /* Freeing a non-alloc`ed signal, i.e. -1, is harmless */
  FreeSignal( AmiGUSBase->agb_MainSignal );
  AmiGUSBase->agb_MainSignal = -1;
  LOG_D(("D: Free`ed main signal\n"));

  /*
   * ------------------------------------------------------
   * Part 1: De-allocate this AmiGUS,
   *         re-entrance/interrupt/multitasking safe.
   * ------------------------------------------------------
   */
  Disable();
  if ( !AmiGUSBase->agb_UsageCounter ) {

    Enable();
    DisplayError( EDriverNotInUse );
    return;
  
  } else {

    --AmiGUSBase->agb_UsageCounter;
    Enable();
  }
  LOG_D(("D: Free`ed AmiGUS AHI hardware\n"));

  LOG_D(("D: AHIsub_FreeAudio done\n"));

  return;
}
