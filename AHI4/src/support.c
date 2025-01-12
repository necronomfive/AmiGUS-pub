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

#include <devices/timer.h>
#include <exec/alerts.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/timer.h>
#include <proto/utility.h>

#include "amigus_ahi_sub.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

/**
 * Type for error message declarations.
 */
struct TErrorMessage
  {
  ULONG iError;
  UBYTE* iMessage;
  UBYTE* iButton;
  };

/**
 * Mapping between error code, error message and button label.
 */
struct TErrorMessage errors[] =
  {
    { ENoError, "", "" },

    /* Insert errors below. */
    { EOpenDosBase, "Can not open dos.library.", "What the ...?" },
    { EOpenUtilityBase, "Your utility.library is shit!", "I'm sorry." },
    { EGetAttrNotImplemented, "AHI requested unknown info, may work still...", "Fingers crossed" },

    { EAmiGUSNotFound, "AmiGUS card not found.", "Read?" },
    { EAmiGUSDetectError, "AmiGUS card detection mess.", "Damn!" },
    { EAmiGUSFirmwareOutdated, "AmiGUS card firmware outdated.", "Will update, promised!" },

    { EAudioModeNotImplemented, "Right now implemented, 16bit, stereo, no Hifi", "Coming soon..." },

    { EOpenLogFile, "Can not create log file RAM:AmiGUS-AHI.log.", "Oops!" },
    { EAllocateLogMem, "Can not allocate memory blob for extra-dirty logging.", "Meh." },
    { EDriverInUse, "Currently only one client is supported.", "Shame!" },
    /*
    { EOutOfMemory, "You are out of memory!", "Shit!" },
    { EMixerBufferNotAligned, "Misaligned mixer buffer not yet handled.", "Ok" },
*/
    { ESampleFormatMissingFromMode, "AMIGUS mode file issue: Lacking AmiGUS_SampleFormat." "Will report issue!" },
    { ECopyFunctionMissingFromMode, "AMIGUS mode file issue: Lacking AmiGUS_CopyFunction." "Will report issue!" },
    { EWorkerProcessCreationFailed, "Could not create playback worker.", "Damn." },
    { EWorkerProcessDied, "Playback worker died.", "RIP" },
    { EWorkerProcessSignalsFailed, "Worker does not like to communicate.", "Swine!" },
    { EMainProcessSignalsFailed, "Main process is deaf-mute, this won't work.", "Oh." },

    { ERecordingNotImplemented, "This driver does not support recording.", "Fine!" },
    { ERecordingModeNotSupported, "The selected audio mode does not support recording.", "Understood" },

    /* Insert errors above. */

    { EUnknownError, "Unknown error.", "Shit!" }
  };

/*
 * Displays an error message, showing the error code and
 * a error message defined in errors[]. If a code can not 
 * be resolved, the EUnknownError text is displayed.
 */
VOID DisplayError( ULONG aError ) {

  ULONG i = ENoError;
  STRPTR message = errors[ ENoError ].iMessage;

  if ( !aError ) {
    return;
  }

  while (( aError != errors[ i ].iError ) &&
         ( EUnknownError != errors[ i ].iError )
        ) {
    i++;
  }
  message = errors[ i ].iMessage;

  if ( IntuitionBase ) {
    struct EasyStruct req;

    req.es_StructSize = sizeof( struct EasyStruct );
    req.es_Flags = 0;
    req.es_Title = LIBRARY_NAME;
    req.es_TextFormat = "Error %ld : %s";
    req.es_GadgetFormat = errors[ i ].iButton;

    EasyRequest( NULL, &req, NULL, aError, message );

  } else {

    Alert( AN_Unknown | AG_OpenLib | AO_Unknown );
  }
  LOG_E(("E: AmiGUS %ld - %s\n", aError, message));
}

VOID LogTicks( UBYTE bitmask ) {

  struct EClockVal ecv;
  ULONG ef = ReadEClock( &ecv );

  switch (bitmask) {
    case 0x00:
      break;
    case 0x01:
      LOG_I(("I: Tick frequency %ld\n", ef));
      break;
    case 0x02:
      LOG_I(("I: Tick low %lu\n", ecv.ev_lo));
      break;
    case 0x04:
      LOG_I(("I: Tick high %lu\n", ecv.ev_hi));
      break;
    case 0x03:
      LOG_I(("I: Tick freq %ld low %ld\n", ef, ecv.ev_lo));
      break;
    case 0x07:
      LOG_I(("I: Tick freq %ld low %ld high %ld\n", ef, ecv.ev_lo, ecv.ev_hi));
      break;
  }
}

VOID Sleep( ULONG millis ) {

  struct EClockVal ecv;
  ULONG ef = ReadEClock( &ecv );
  ULONG milli_ticks = UDivMod32( ef, 1000 );           // ef -> ticks / milli
  ULONG needed_ticks = UMult32( milli_ticks, millis ); //    -> target inc
  /*
   * Higher precision, but 32bit not wide enough:
   * ULONG second_ticks = UMult32( ef, millis );
   * ULONG needed_ticks = UDivMod32( second_ticks, 1000 );
  */
  const ULONG current_lo = ecv.ev_lo;
  const ULONG current_hi = ecv.ev_hi;
  const ULONG target_lo = current_lo + needed_ticks;
  const ULONG target_hi = ( current_lo > target_lo )
                          ? ( current_hi + 1 )
                          : ( current_hi );

  LOG_V(( "V: Tick freq %ld current: %ld %ld needed: %ld target: %ld %ld\n",
          ef,
          current_hi,
          current_lo,
          needed_ticks,
          target_hi,
          target_lo ));
  while (( target_lo > ecv.ev_lo ) || ( target_hi > ecv.ev_hi )) {

    ReadEClock( &ecv );
  }
  LOG_V(( "V: Slept until %ld %ld\n", ecv.ev_hi, ecv.ev_lo ));
}