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

#ifndef IDCMP_GADGETUP
#define IDCMP_GADGETUP 0x00000040
#endif /* IDCMP_GADGETUP */

#ifndef INCLUDE_VERSION
/* Patches to make this compile in NDK1.3 */
#define AN_Unknown                0x35000000
#define AO_Unknown                0x00008035

#define AlertCompat( x )          Alert( x, NULL )

#else

#define AlertCompat( x )          Alert( x )

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

VOID ShowError_v34( STRPTR title, STRPTR message, STRPTR button, LONG error );
VOID ShowError_v36( STRPTR title, STRPTR message, STRPTR button, LONG error );

/*
 * Displays an error message, showing the error code and
 * a error message defined in errors[]. If a code can not 
 * be resolved, the EUnknownError text is displayed.
 */
VOID DisplayError( ULONG error ) {

  ULONG i = ENoError;

  if ( !error ) {
    return;
  }

  while (( error != errors[ i ].iError ) &&
         ( EUnknownError != errors[ i ].iError )
        ) {
    i++;
  }

  if ( IntuitionBase ) {

    STRPTR title = STR( LIB_FILE );
    STRPTR message = errors[ i ].iMessage;
    STRPTR button = errors[ i ].iButton;
#ifdef INCLUDE_VERSION
    if ( 36 > IntuitionBase->LibNode.lib_Version ) {
#endif

      ShowError_v34( title, message, button, error );

#ifdef INCLUDE_VERSION
    } else {

      ShowError_v36( title, message, button, error );
    }
#endif
  } else {

    AlertCompat( AN_Unknown | AG_OpenLib | AO_Unknown );
  }
  LOG_E(( "E: AmiGUS %ld - %s\n", error, errors[ i ].iMessage ));
}

VOID NonConflictingNewMinList( struct MinList * list ) {

  list->mlh_Head = ( struct MinNode * ) &list->mlh_Tail;
  list->mlh_Tail = NULL;
  list->mlh_TailPred = ( struct MinNode * ) list;
}

VOID ShowError_v34( STRPTR title, STRPTR message, STRPTR button, LONG error ) {

  struct IntuiText body;
  struct IntuiText negative;
  struct Window * window;

  LOG_V(( "V: Using V34 requester\n" ));
  body.BackPen = 0;
  body.FrontPen = 2;
  body.DrawMode = 0;
  body.ITextFont = NULL;
  body.LeftEdge = 8;
  body.TopEdge = 5;
  body.IText = message;
  body.NextText = NULL;

  negative.BackPen = 0;
  negative.FrontPen = 2;
  negative.DrawMode = 0;
  negative.ITextFont = NULL;
  negative.LeftEdge = 6;
  negative.TopEdge = 3;
  negative.IText = button;
  negative.NextText = NULL;

  window = BuildSysRequest(
    ( struct Window * ) NULL,         // parent Window
    ( struct IntuiText * ) &body,     // body IntuiText
    ( struct IntuiText * ) NULL,      // positive IntuiText
    ( struct IntuiText * ) &negative, // negative IntuiText
    ( ULONG ) IDCMP_GADGETUP,         // flags
    ( UWORD ) 640,                    // width
    ( UWORD ) 50                      // height
  );
  SetWindowTitles( window, title, NULL /*"screen title"*/ );
  Wait( 1L << window->UserPort->mp_SigBit );
  FreeSysRequest( window );
}

VOID ShowError_v36( STRPTR title, STRPTR message, STRPTR button, LONG error ) {

#ifdef INCLUDE_VERSION
  struct EasyStruct req;

  LOG_V(( "V: Using V36 requester\n" ));
  req.es_StructSize = sizeof( struct EasyStruct );
  req.es_Flags = 0;
  req.es_Title = title;
  req.es_TextFormat = "Error %ld : %s";
  req.es_GadgetFormat = button;

  EasyRequest( NULL, &req, NULL, error, message );
#endif
}
