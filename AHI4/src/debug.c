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

 #include <proto/dos.h>
#include <proto/exec.h>

#include "SDI_compiler.h"

#include "amigus_ahi_sub.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

/******************************************************************************
 * Debug helper functions - private functions.
 *****************************************************************************/

#if defined( FILE_LOG ) | defined( MEM_LOG )

/**
 * Puts a single character to a location pointed to,
 * moving the location forward afterwards.
 * Core of memory and file debug prints below.
 *
 * @param c Character to place.
 * @param target Pointer to the target location pointer.
 */
ASM( VOID ) debug_mPutChProc( REG( d0, UBYTE c ), REG( a3, UBYTE ** target )) {

  **target = c;
  ++( *target );
}

#endif

/******************************************************************************
 * Debug helper functions - public function definitions.
 *****************************************************************************/

#define NOT_USE_RawPutCharC
#ifdef USE_RawPutCharC

// Comes with a tiny performance impact,
// so... rather not do it.
ASM( VOID ) RawPutCharC( REG( d0, UBYTE putCh ) {

  RawPutChar(putCh);
}

VOID debug_kprintf( STRPTR format, ... ) {

  RawIOInit();
  RawDoFmt( format, 0, ( VOID ( * )()) &RawPutCharC, 0 );
  RawMayGetChar();
} 

#else

VOID debug_kvprintf( STRPTR format, APTR vargs ) {

  RawIOInit();
  // Ugly like hell, but...
  RawDoFmt(
    format,
    vargs,
    /*
    /-------------+-------------------------> Cast void(*)() function pointer
    |             |  /------+---------------> Address math in LONG is better
    |             |  |      | /-----+-------> Exec kick1.2 functions -> SysBase
    |             |  |      | |     | /---+-> RawPutChar is -516 (see above)
    |             |  |      | |     | |   |   */
    ( VOID ( * )()) (( LONG ) SysBase - 516 ),
    0 );
  RawMayGetChar();
}

VOID debug_kprintf( STRPTR format, ... ) {

  debug_kvprintf(
    format,
    /*
    /------+-----------------------> Cast to required APTR 
    |      |  /------+-------------> Nice math in LONGs to make it work 
    |      |  |      | /-----+-----> STRPTR, address of format, first argument
    |      |  |      | |     | /-+-> 4 bytes later, the next argument follows
    |      |  |      | |     | | |   */
    ( APTR ) (( LONG ) &format + 4 ));
}

#endif

#ifdef FILE_LOG

VOID debug_fprintf( STRPTR format, ... ) {

  static BOOL errorShown = FALSE;
  STRPTR logFilePath = "ram:AmiGUS-AHI.log";
  UBYTE buffer[512];
  UBYTE * printBuffer = buffer;

  if (( !SysBase ) || ( !DOSBase )) {

    debug_kprintf( "E: Tried sending log to file before opening libs!\n" );
    return;
  }

  if ( !AmiGUS_AHI_Base->agb_LogFile ) {
    if ( errorShown ) {

      return;
    }

#ifdef INCLUDE_VERSION
    if ( 36 <= (( struct Library *) DOSBase )->lib_Version ) {

      LONG i = GetVar( "AmiGUS-AHI-LOG-FILEPATH",
                       buffer,
                       sizeof( buffer ),
                       0 );
      if (( i > 0 ) && ( i < 512 )) {

        logFilePath = buffer;
      }
    }
#endif

    AmiGUS_AHI_Base->agb_LogFile = Open( logFilePath, MODE_NEWFILE );

    if ( !AmiGUS_AHI_Base->agb_LogFile ) {

      errorShown = TRUE;
      DisplayError( EOpenLogFile );
      return;
    }
  }
  RawDoFmt(
    format,
    /*
    /------+-----------------------> Cast to required APTR 
    |      |  /------+-------------> Nice math in LONGs to make it work 
    |      |  |      | /-----+-----> STRPTR, address of format, first argument
    |      |  |      | |     | /-+-> 4 bytes later, the next argument follows
    |      |  |      | |     | | |   */
    ( APTR ) (( LONG ) &format + 4 ),
    &debug_mPutChProc,
    &printBuffer );
  Write( AmiGUS_AHI_Base->agb_LogFile, buffer, printBuffer - buffer - 1 );
}

#endif /* FILE_LOG */
#ifdef MEM_LOG

VOID debug_mprintf( STRPTR format, ... ) {

  /*
   * Used to ensure to only try allocating buffer exactly once,
   * even if it fails.
   */
  static BOOL errorShown = FALSE;
  const STRPTR memMarker[] = {

    AMIGUS_MEM_LOG_BORDERS,
    STR( LIB_FILE ),
    AMIGUS_MEM_LOG_BORDERS
  };

  if ( !AmiGUS_AHI_Base->agb_LogMem ) {

    // Yep, defaults to 
    LONG size = 32 << 20;             // 32MB somewhere
    APTR where = ( APTR ) 0;          // in 3/4000 CPU board space

    if ( errorShown ) {

      return;
    }    

#ifdef INCLUDE_VERSION
    if ( 36 <= (( struct Library * ) DOSBase )->lib_Version) {

      UBYTE buffer[ 64 ];
      LONG i = GetVar( "AmiGUS-AHI-LOG-ADDRESS", buffer, sizeof( buffer ), 0 );
      /*
       * UAE:  setenv AmiGUS-AHI-LOG-ADDRESS 1207959552 -> 0x48000000
       * 3/4k: setenv AmiGUS-AHI-LOG-ADDRESS 167772160  -> 0x0a000000
       * 2k:   setenv AmiGUS-AHI-LOG-ADDRESS 4194304    -> 0x00400000
       */

      if ( i > 0 ) {

        StrToLong( buffer, ( LONG * ) &where );
      }

      i = GetVar( "AmiGUS-AHI-LOG-SIZE", buffer, sizeof( buffer ), 0 );
      if ( i > 0 ) {

        StrToLong( buffer, &size );
      }
    }
#endif

    debug_kprintf( "AmiGUS-AHI-LOG-ADDRESS %lx = %ld (requested)\n"
                   "AmiGUS-AHI-LOG-SIZE %ld\n",
                   ( LONG ) where,
                   ( LONG ) where,
                   size );

    if ( 0 < ( LONG ) where ) {

      AmiGUS_AHI_Base->agb_LogMem = AllocAbs( size, where );
    }
    if ( !AmiGUS_AHI_Base->agb_LogMem ) {

      AmiGUS_AHI_Base->agb_LogMem = AllocAbs( size, ( APTR ) 0x0a000000 );
    }
    if ( !AmiGUS_AHI_Base->agb_LogMem ) {

      AmiGUS_AHI_Base->agb_LogMem = AllocAbs( size, ( APTR ) 0x00400000 );
    }
    if ( !AmiGUS_AHI_Base->agb_LogMem ) {

      AmiGUS_AHI_Base->agb_LogMem = AllocAbs( size, ( APTR ) 0x48000000 );
    }
    if ( !AmiGUS_AHI_Base->agb_LogMem ) {

      size = ( 2 << 19 ) - 4;
      AmiGUS_AHI_Base->agb_LogMem = AllocAbs( size, ( APTR ) 0x00400000 );
    }
    if ( AmiGUS_AHI_Base->agb_LogMem ) {

      LONG i;
      for ( i = 0; i < size; i += 4 ) {
        *( ULONG * )(( LONG ) AmiGUS_AHI_Base->agb_LogMem + i ) = 0;
      }
    } else {

      AmiGUS_AHI_Base->agb_LogMem = AllocMem( size, MEMF_CLEAR | MEMF_PUBLIC );
    }
    if ( !AmiGUS_AHI_Base->agb_LogMem ) {

      debug_kprintf( "AmiGUS Log giving up...\n" );
      errorShown = TRUE;
      DisplayError( EAllocateLogMem );
      return;
    }
    debug_kprintf( "AmiGUS Log @ 0x%08lx = %ld (retrieved), size %ld\n",
                   ( LONG ) AmiGUS_AHI_Base->agb_LogMem,
                   ( LONG ) AmiGUS_AHI_Base->agb_LogMem,
                   size );

    RawDoFmt( "%s %s %s\n",
              ( APTR ) memMarker,
              &debug_mPutChProc,
              &AmiGUS_AHI_Base->agb_LogMem );
    /* Move mem blob pointer back to overwrite trailing zero next comment */
    AmiGUS_AHI_Base->agb_LogMem =
      ( APTR )(( ULONG ) AmiGUS_AHI_Base->agb_LogMem - 1 );
    debug_kprintf( "AmiGUS Log ready\n" );
  }

  RawDoFmt(
    format,
    /*
    /------+-----------------------> Cast to required APTR 
    |      |  /------+-------------> Nice math in LONGs to make it work 
    |      |  |      | /-----+-----> STRPTR, address of format, first argument
    |      |  |      | |     | /-+-> 4 bytes later, the next argument follows
    |      |  |      | |     | | |   */
    ( APTR ) (( LONG ) &format + 4 ),
    &debug_mPutChProc,
    &AmiGUS_AHI_Base->agb_LogMem );
  /* Move mem blob pointer back to overwrite trailing zero next comment */
  AmiGUS_AHI_Base->agb_LogMem =
    ( APTR )(( ULONG ) AmiGUS_AHI_Base->agb_LogMem - 1 );
}

#endif /* MEM_LOG */
