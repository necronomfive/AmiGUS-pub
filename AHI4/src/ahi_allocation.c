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
#include "utilities.h"
#include "SDI_AHI4_protos.h"

/* Basic functions - Allocation */

ASM(ULONG) SAVEDS AHIsub_AllocAudio(
//  REG(a6, struct Library* aBase) works fine with vbcc, but not with SASC,
  REG(a1, struct TagItem* aTagList),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
  struct TagItem *stateTag = 0;
  struct TagItem *tag = 0;
  ULONG sampleRateId = 0;
  ULONG sampleRate = 0;
  WORD sampleFormat = -1;
  UBYTE isStereo = FALSE;
  UBYTE isHifi = FALSE;
  UBYTE isRealtime = FALSE;
  UBYTE bitsPerAmiGusSample = 0;
  UWORD ahiBufferMultipleOf;
  UWORD ahiSampleBytes;
  UWORD ahiBufferBytes;
  UWORD ahiBufferSamples;

  /* 
   * Will rely on AHI provided mixing,
   * will do own timing to be able to adjust buffer sizes.
   */
  ULONG result = AHISF_MIXING;

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
  sampleRateId = FindSampleRateIdForValue( aAudioCtrl->ahiac_MixFreq );
  sampleRate = FindSampleRateValueForId( sampleRateId );
  LOG_V(("D: Using %ldHz = 0x%02lx for requested %ldHz\n",
         sampleRate,
         sampleRateId,
         aAudioCtrl->ahiac_MixFreq));
  aAudioCtrl->ahiac_MixFreq = sampleRate;
  AmiGUSBase->agb_SampleRateId = sampleRateId;

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
        result |= ( isStereo << AHISB_KNOWSTEREO );
        break;
      }
      case AHIDB_HiFi: {
        isHifi = (UBYTE)tag->ti_Data;
        result |= ( isHifi << AHISB_KNOWHIFI );
        break;
      }
      case AHIDB_Realtime: {
        isRealtime = (UBYTE)tag->ti_Data;
        break;
      }
      case AHIDB_AmiGUS_SampleFormat: {
        sampleFormat = (WORD)tag->ti_Data;
        break;
      }
      default: {
        break;
      }
    }
  }

  if ( 0 < sampleFormat ) {

    DisplayError( ESampleFormatMissingFromMode );
    return AHISF_ERROR;
  }

  AmiGUSBase->agb_SampleFormat = sampleFormat;
  LOG_I(( "I: AmiGUS mode is format %ld, "
          "%ldbit, %ld stereo, %ld HiFi, %ld Realtime, %ldHz\n",
          AmiGUSBase->agb_SampleFormat,
          bitsPerAmiGusSample, isStereo, isHifi, isRealtime, sampleRate ));

  /*
   * ------------------------------------------------------
   * Part 3: Apply information to AmiGUS & driver.
   * ------------------------------------------------------
   */
  if ( isHifi ) {

    ahiSampleBytes = 4;
    if ( 16 == bitsPerAmiGusSample ) {

      AmiGUSBase->agb_CopyFunction = &Shift16LongCopy;
      ahiBufferMultipleOf = 8;

    } else {

      AmiGUSBase->agb_CopyFunction = &Merge24LongCopy;
      ahiBufferMultipleOf = 16;
    }
  } else {

    AmiGUSBase->agb_CopyFunction = &PlainLongCopy;
    ahiBufferMultipleOf = 4;

    if ( 16 == bitsPerAmiGusSample ) {

      ahiSampleBytes = 2;

    } else {

      ahiSampleBytes = 1;
    }
  }
  if ( isStereo ) {

    ahiSampleBytes <<= 1;
  }

  LOG_D(( "D: Applying properties: "
          "sample rate %ld ~ size %ld buf mult %ld stereo %ld realtime %ld\n",
          sampleRate,
          ahiSampleBytes,
          ahiBufferMultipleOf,
          isStereo,
          isRealtime ));
  ahiBufferBytes = getBufferBytes(
    sampleRate,
    ahiSampleBytes,
    ahiBufferMultipleOf,
    isStereo,
    isRealtime );
  ahiBufferSamples = getBufferSamples(
    ahiBufferBytes,
    ahiSampleBytes,
    isStereo );
  LOG_I(( "I: Setting mixing buffer properties to %ld samples,"
          " %ld bytes, %ld longs and watermark %ld WORDs\n",
          ahiBufferSamples,
          ahiBufferBytes,
          ahiBufferBytes >> 2,
          ahiBufferBytes >> 1 ));

  aAudioCtrl->ahiac_MixFreq = sampleRate;
  aAudioCtrl->ahiac_BuffSamples = ahiBufferSamples;

  /* Buffers are ticking in LONGs! */
  AmiGUSBase->agb_BufferMax = ahiBufferBytes >> 2;
  /* Watermark is ticking in WORDs! */
  if ( ( AMIGUS_PLAYBACK_FIFO_WORDS >> 1 ) > ( ( ahiBufferBytes >> 1 )) ) {

    AmiGUSBase->agb_watermark = ahiBufferBytes >> 1;

  } else {

    AmiGUSBase->agb_watermark = AMIGUS_PLAYBACK_FIFO_WORDS >> 1;
  }

  LOG_D(( "D: Mix %ld samples = %ld LONGs per pass, watermark %ld WORDs\n",
          ahiBufferSamples,
          AmiGUSBase->agb_BufferMax,
          AmiGUSBase->agb_watermark ));

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

  LOG_D(( "D: AHIsub_AllocAudio done, returning %ld.\n", result ));

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
