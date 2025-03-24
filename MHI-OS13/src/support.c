/*
 * This file is part of the mhiAmiGUS.library.
 *
 * mhiAmiGUS.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <exec/types.h>
#include <devices/timer.h>
#include <exec/alerts.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/timer.h>

#include "amigus_mhi.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

#ifndef INCLUDE_VERSION
/* Patches to make this compile in NDK1.3 */
#define AN_Unknown                0x35000000
#define AO_Unknown                0x00008035
#endif

/**
 * Type for error message declarations.
 */
struct TErrorMessage {
  ULONG iError;
  UBYTE* iMessage;
  UBYTE* iButton;
};

/**
 * Mapping between error code, error message and button label.
 */
struct TErrorMessage errors[] = {
  { ENoError, "", "" },

  /* Insert errors below. */
  { ELibraryBaseInconsistency, "Library memory somehow damaged, expecting a crash soon.", "Have a nice day!" },
  { EOpenDosBase, "Can not open dos.library.", "What the ...?" },
  { EOpenUtilityBase, "Your utility.library is shit!", "I'm sorry." },
  { EGetAttrNotImplemented, "AHI requested unknown info, may work still...", "Fingers crossed" },

  { EAmiGUSNotFound, "AmiGUS card not found.", "Read?" },
  { EAmiGUSDetectError, "AmiGUS card detection mess.", "Damn!" },
  { EAmiGUSInUseError, "AmiGUS codec part is in use by another driver.", "Oops." },
  { EAmiGUSFirmwareOutdated, "AmiGUS card firmware outdated.", "Will update, promised!" },

  { EAudioModeNotImplemented, "Right now implemented, 16bit, stereo, no Hifi", "Coming soon..." },

  { EOpenLogFile, "Can not create log file RAM:AmiGUS-MHI.log.", "Oops!" },
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

  if ( !aError ) {
    return;
  }

  while (( aError != errors[ i ].iError ) &&
         ( EUnknownError != errors[ i ].iError )
        ) {
    i++;
  }

  if ( IntuitionBase ) {

    ShowError( STR( LIB_FILE ), errors[ i ].iMessage, errors[ i ].iButton );

  } else {

    ShowAlert( AN_Unknown | AG_OpenLib | AO_Unknown );
  }
  LOG_E(( "E: AmiGUS %ld - %s\n", aError, errors[ i ].iMessage ));
}

VOID NonConflictingNewMinList( struct MinList * list ) {

  list->mlh_Head = ( struct MinNode * ) &list->mlh_Tail;
  list->mlh_Tail = NULL;
  list->mlh_TailPred = ( struct MinNode * ) list;
}

struct MsgPort * CreatePort( BYTE * name, LONG priority ) {

  struct MsgPort * result;
  BYTE signal = AllocSignal( -1L );

  if ( -1 == signal ) {

    return NULL;
  }

  result = ( struct MsgPort * ) AllocMem( sizeof( struct MsgPort ),
                                          MEMF_PUBLIC | MEMF_CLEAR );
  if ( !result ) {

    FreeSignal( signal );
    return NULL;
  }
  result->mp_Node.ln_Name = name;
  result->mp_Node.ln_Pri  = priority;
  result->mp_Node.ln_Type = NT_MSGPORT;
  result->mp_Flags        = PA_SIGNAL;
  result->mp_SigBit       = signal;
  result->mp_SigTask      = ( struct Task * ) FindTask( 0 ); // Current task!
  if ( name ) {

    AddPort(result);

  } else {

    NonConflictingNewMinList(( struct MinList * )&( result->mp_MsgList ));
  }
  return result;
}

VOID DeletePort( struct MsgPort * port ) {

  if ( !port ) {

    return;
  }
  if ( port->mp_Node.ln_Name ) {

    RemPort(port);
  }
  if (( port->mp_Flags & PF_ACTION ) == PA_SIGNAL ) {

    FreeSignal( port->mp_SigBit );
    port->mp_Flags = PA_IGNORE;
  }
  FreeMem( port, sizeof( struct MsgPort ));
}

struct IORequest * CreateExtIO( struct MsgPort * port, ULONG ioSize ) {

  struct IORequest * result = NULL;
  if ( port ) {
    result = AllocMem( ioSize, MEMF_PUBLIC | MEMF_CLEAR );
    if ( result ) {
        result->io_Message.mn_ReplyPort = port;
        result->io_Message.mn_Length = ioSize;
        result->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
    }
  }
  return( result );
}

VOID DeleteExtIO( struct IORequest * ioReq ) {

  if ( ioReq ) {

    ioReq->io_Message.mn_Node.ln_Succ = NULL;
    ioReq->io_Device = NULL;
    FreeMem( ioReq, ioReq->io_Message.mn_Length);
  }
}
