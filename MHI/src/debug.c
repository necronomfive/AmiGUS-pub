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

#include <proto/dos.h>
#include <proto/exec.h>

#include "SDI_compiler.h"

#include "amigus_mhi.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

#define NOT_USE_RawPutCharC
#ifdef USE_RawPutCharC

// Comes with a tiny performance impact,
// so... rather not do it.
ASM(VOID) RawPutCharC( REG(d0, UBYTE putCh ) {

  RawPutChar(putCh);
}

VOID debug_kprintf( STRPTR format, ... ) {

  RawIOInit();
  RawDoFmt( format, 0, (VOID (*)())&RawPutCharC, 0 );
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
    /----------+----------------------> Cast to void(*)() function pointer
    |          |  /----+--------------> Address math in LONG works better
    |          |  |    | /-----+------> Exec kick1.2 function using SysBase
    |          |  |    | |     | /--+-> RawPutChar is -516 (see above)
    |          |  |    | |     | |  |   */
    (VOID (*)()) ((LONG) SysBase -516),
    0 );
  RawMayGetChar();
}

VOID debug_kprintf( STRPTR format, ... ) {

  debug_kvprintf(
    format,
    /*
    /----+--------------------> Cast to required APTR 
    |    |  /----+------------> Nice math in LONGs to make it work 
    |    |  |    | /-----+----> STRPTR, address of format, first argument
    |    |  |    | |     | /+-> 4 bytes later, the next argument follows
    |    |  |    | |     | ||   */
    (APTR) ((LONG) &format +4) );
}

#endif

#ifdef USE_FILE_LOGGING

VOID debug_fprintf( STRPTR format, ... ) {

  static BOOL errorShown = FALSE;
  STRPTR logFilePath = "ram:AmiGUS-MHI.log";
  UBYTE buffer[512];
  LONG i;
  
  i = GetVar( "AmiGUS-MHI-LOG-FILEPATH", buffer, sizeof(buffer), 0 );
  if (( i > 0 ) && (i <= 512 )) {
    logFilePath = buffer;
  }
  if (( !AmiGUSBase->agb_LogFile ) || ( errorShown )) {
    AmiGUSBase->agb_LogFile = Open( logFilePath, MODE_NEWFILE );
    if ( !AmiGUSBase->agb_LogFile ) {
      DisplayError( EOpenLogFile );
      errorShown = TRUE;
      return;
    }
  }
  VFPrintf(
    AmiGUSBase->agb_LogFile,
    format,
    /*
    /----+--------------------> Cast to required APTR 
    |    |  /----+------------> Nice math in LONGs to make it work 
    |    |  |    | /-----+----> STRPTR, address of format, first argument
    |    |  |    | |     | /+-> 4 bytes later, the next argument follows
    |    |  |    | |     | ||   */
    (APTR) ((LONG) &format +4) );
}

#endif /* USE_FILE_LOGGING */
#ifdef USE_MEM_LOGGING

ASM(VOID) debug_mPutChProc( REG(d0, UBYTE c), REG(a3, UBYTE ** target) ) {

  **target = c;
  ++(*target);
}

VOID debug_mprintf( STRPTR format, ... ) {

  /*
   * Used to ensure to only try allocating buffer exactly once,
   * even if it fails.
   */
  static BOOL attempted = FALSE;
  
  if ( !AmiGUSBase->agb_LogMem ) {

    // Yep, defaults to 
    LONG size = 32<<20;             // 32MB somewhere
    APTR where = (APTR) 0x0a000000; // in 3/4000 CPU board space
    UBYTE buffer[64];
    LONG i;

    if ( attempted ) {

      debug_kprintf( "AmiGUS Log gave up...\n" );
      return;
    }    
    attempted = TRUE;

    i = GetVar( "AmiGUS-MHI-LOG-ADDRESS", buffer, sizeof(buffer), 0 );
    if ( i > 0 ) {
      StrToLong( buffer, (LONG *) &where );
    }
    /*
     * UAE:  setenv AmiGUS-MHI-LOG-ADDRESS 1207959552 -> 0x48000000
     * 3/4k: setenv AmiGUS-MHI-LOG-ADDRESS 167772160  -> 0x0a000000
     */
    i = GetVar( "AmiGUS-MHI-LOG-SIZE", buffer, sizeof(buffer), 0 );
    if ( i > 0 ) {
      StrToLong( buffer, &size );
    }

    debug_kprintf(
      "AmiGUS-MHI-LOG-ADDRESS %lx = %lu (requested)\nAmiGUS-MHI-LOG-SIZE %ld\n", 
      (ULONG)where,
      (ULONG)where,
      size
    );

    if ( 0 < (LONG) where ) {
      AmiGUSBase->agb_LogMem = AllocAbs( size, where );
      if ( AmiGUSBase->agb_LogMem ) {
        for ( i = 0; i < size; i += 4 ) {
          *(ULONG *)((LONG) where + i) = 0;
        }
      }
    } else {
      AmiGUSBase->agb_LogMem = AllocMem(
        size, 
        MEMF_CLEAR | MEMF_PUBLIC );
    }
    if ( !AmiGUSBase->agb_LogMem ) {
    
      DisplayError( EAllocateLogMem );
      debug_kprintf( "AmiGUS Log giving up...\n" );
      return;
    }
    debug_kprintf(
      "AmiGUS Log @ 0x%lx = %lu (retrieved)\n",
      AmiGUSBase->agb_LogMem,
      AmiGUSBase->agb_LogMem
    );
    RawDoFmt(
      AMIGUS_MEM_LOG_MARKER,
      NULL,
      &debug_mPutChProc,
      &AmiGUSBase->agb_LogMem );
    /* Move mem blob pointer back to overwrite trailing zero next comment */
    AmiGUSBase->agb_LogMem = ( APTR )(( ULONG ) AmiGUSBase->agb_LogMem - 1 );
    debug_kprintf( "AmiGUS Log ready\n" );
  }

  RawDoFmt(
    format,
    /*
    /----+--------------------> Cast to required APTR 
    |    |  /----+------------> Nice math in LONGs to make it work 
    |    |  |    | /-----+----> STRPTR, address of format, first argument
    |    |  |    | |     | /+-> 4 bytes later, the next argument follows
    |    |  |    | |     | ||   */
    (APTR) ((LONG) &format +4),
    &debug_mPutChProc,
    &AmiGUSBase->agb_LogMem );
  /* Move mem blob pointer back to overwrite trailing zero next comment */
  AmiGUSBase->agb_LogMem = ( APTR )(( ULONG ) AmiGUSBase->agb_LogMem - 1 );
}

#endif /* USE_MEM_LOGGING */
