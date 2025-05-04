/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/utility.h>

#include "amigus_ahi_modes.h"
#include "amigus_ahi_sub.h"
#include "amigus_hardware.h"
#include "debug.h"
#include "errors.h"
#include "samplerate.h"
#include "support.h"
#include "SDI_ahi_sub.h"

/* Basic functions - Allocation */

ASM(ULONG) SAVEDS AHIsub_AllocAudio(
  REG(a1, struct TagItem* aTagList),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
  struct TagItem *stateTag = 0;
  struct TagItem *tag;
  ULONG sampleRateId;
  ULONG sampleRate;
  ULONG modeId = -1;
  UBYTE modeOffset;
  struct PlaybackProperties * playProps;
  struct RecordingProperties * recProps;
  UBYTE isStereo = FALSE;
  UBYTE isHifi = FALSE;
  UBYTE isRealtime = FALSE;
  UBYTE canRecord = FALSE;
  UBYTE bitsPerAmiGusSample = 0;

  /* 
   * Will rely on AHI provided mixing and timing,
   * trying out AHIsub_AllocAudio -> 1.
   */
  ULONG result = AHISF_MIXING | AHISF_TIMING;

  LOG_D(( "D: AHIsub_AllocAudio start\n" ));
  LogTicks( 0x03 );

  /*
   * ------------------------------------------------------
   * Part 1: Allocate this AmiGUS to exactly this callee,
   *         re-entrance/interrupt/multitasking safe.
   * ------------------------------------------------------
   */
  Disable();
  if ( AmiGUSBase->agb_UsageCounter ) {

    // TODO: Allow 2 clients here - 1 for recording, 1 for playback! Need to be super independent though and some rework before...
    Enable();
    DisplayError( EDriverInUse );
    return AHISF_ERROR;
  
  } else {

    ++AmiGUSBase->agb_UsageCounter;
    Enable();
  }
  // Magic: AudioControl of client "owns" this card :)
  aAudioCtrl->ahiac_DriverData = AmiGUSBase->agb_CardBase;
  LOG_D(("D: Alloc`ed AmiGUS AHI hardware\n"));

  /*
   * ------------------------------------------------------
   * Part 2: Extract audio mode information
   * ------------------------------------------------------
   */
  /* Find nearest supported frequency and provide it back */
  sampleRateId = FindSampleRateIdForValue( aAudioCtrl->ahiac_MixFreq );
  sampleRate = FindSampleRateValueForId( sampleRateId );
  LOG_I(("D: Using %ldHz = 0x%02lx for requested %ldHz\n",
         sampleRate,
         sampleRateId,
         aAudioCtrl->ahiac_MixFreq));
  aAudioCtrl->ahiac_MixFreq = sampleRate;

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
        if ( isStereo ) {

          result |= AHISF_KNOWSTEREO;
        }
        break;
      }
      case AHIDB_HiFi: {
        isHifi = ( UBYTE )tag->ti_Data;
        if ( isHifi ) {

          result |= AHISF_KNOWHIFI;
        }
        break;
      }
      case AHIDB_Realtime: {
        isRealtime = ( UBYTE )tag->ti_Data;
        break;
      }
      case AHIDB_AudioID: {
        modeId = ( ULONG )tag->ti_Data;
        break;
      }
      case AHIDB_Record: {
        canRecord = ( UBYTE )tag->ti_Data;
        if ( canRecord ) {

          result |= AHISF_CANRECORD;

        } else {

          result &= ( ~ AHISF_CANRECORD );
        }
        break;
      }
      default: {
        break;
      }
    }
  }

  if (( AMIGUS_AHI_MIN_MODE > modeId ) || ( AMIGUS_AHI_MAX_MODE < modeId )) {

    DisplayError( EAudioModeNotImplemented );
    return AHISF_ERROR;
  }

  /*
   * ------------------------------------------------------
   * Part 3: Apply information to AmiGUS & driver.
   * ------------------------------------------------------
   */
  modeOffset = modeId - AMIGUS_AHI_MIN_MODE;
  AmiGUSBase->agb_AhiModeOffset = modeOffset;
  playProps = &PlaybackPropertiesById[ modeOffset ];
  recProps = &RecordingPropertiesById[ modeOffset ];
  LOG_I(( "I: AHI mode is 0x%08lx, offset %ld, "
          "%ldbit, %ld stereo, %ld HiFi, %ld Realtime, %ldHz\n",
          modeId, modeOffset,
          bitsPerAmiGusSample, isStereo, isHifi, isRealtime, sampleRate ));
  LOG_I(( "I: AmiGUS playback mode format 0x%lx, "
          "AHI samples %ld BYTEs, "
          "conversion samples<->BYTEs %ld shifts, "
          "AmiGUS samples %ld BYTEs\n",
          playProps->pp_HwFormatId,
          ( 1 << playProps->pp_AhiSampleShift ),
          playProps->pp_AhiSampleShift,
          playProps->pp_HwSampleSize ));
  LOG_I(( "I: AmiGUS recording mode format 0x%lx, %ld Recording, "
          "AHI samples %ld BYTEs, "
          "conversion samples<->BYTEs by %ld shifts\n",
          recProps->rp_HwFormatId, canRecord,
          ( 1 << recProps->rp_AhiSampleShift ),
          recProps->rp_AhiSampleShift ));

  AmiGUSBase->agb_HwSampleRateId = sampleRateId;
  AmiGUSBase->agb_CanRecord = canRecord;

  AmiGUSBase->agb_Playback.agpp_HwSampleSize = playProps->pp_HwSampleSize;
  AmiGUSBase->agb_Playback.agpp_CopyFunction = playProps->pp_CopyFunction;

  AmiGUSBase->agb_Recording.agpr_AhiSampleShift = recProps->rp_AhiSampleShift;
  AmiGUSBase->agb_Recording.agpr_CopyFunction = recProps->rp_CopyFunction;
  AmiGUSBase->agb_Recording.agpr_RecordingMessage.ahirm_Type = 
    recProps->rp_AhiFormatId;

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

  LOG_D(( "D: AHIsub_AllocAudio done, returning 0x%lx.\n", result ));

  return result;
}

ASM( VOID ) SAVEDS AHIsub_FreeAudio(
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(("D: AHIsub_FreeAudio start\n"));

  if ( aAudioCtrl->ahiac_DriverData != AmiGUSBase->agb_CardBase ) {

    LOG_W(( "W: Cannot free a card not alloc'ed!\n" ));
    return;
  }

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
  aAudioCtrl->ahiac_DriverData = NULL;
  LOG_D(("D: Free`ed AmiGUS AHI hardware\n"));

  LOG_D(("D: AHIsub_FreeAudio done\n"));

  return;
}
