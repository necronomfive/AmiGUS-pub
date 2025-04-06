/*
 * This licence was adapted from the BSD licence.
 *
 * Copyright © Paul Qureshi, Thomas Whenzel and Dirk Conrad. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the authors nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * Or, in English:
 *     - You're free to derive any work you like from this, just don't change
 *       the original source.
 *     - Give credit where credit is due
 *     - Don't fob it off as your own work
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */
/*****************************************************/
/*                                                   */
/* MHIplay: A simple example how to play             */
/*          multibuffered data via MHI               */
/*                                                   */
/*          Revision 1.0 by Thomas Wenzel            */
/*          Fixed for OS1.3 by Christoph Fassbach    */
/*                                                   */
/*****************************************************/

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>

#include <libraries/dos.h>
#include <libraries/mhi.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/mhi.h>

#include "SDI_compiler.h"

// inlines created by
// fd2pragma SPECIAL 70 CLIB /clib/mhi_protos.h INFILE mhi_lib.fd TO /INLINE/
#ifdef __VBCC__
#ifndef _VBCCINLINE_MHI_H
#define _VBCCINLINE_MHI_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

APTR __MHIAllocDecoder(__reg("a6") struct Library *, __reg("a0") struct Task * task, __reg("d0") ULONG mhisignal)="\tjsr\t-30(a6)";
#define MHIAllocDecoder(task, mhisignal) __MHIAllocDecoder(MHIBase, (task), (mhisignal))

VOID __MHIFreeDecoder(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-36(a6)";
#define MHIFreeDecoder(handle) __MHIFreeDecoder(MHIBase, (handle))

BOOL __MHIQueueBuffer(__reg("a6") struct Library *, __reg("a3") APTR handle, __reg("a0") APTR buffer, __reg("d0") ULONG size)="\tjsr\t-42(a6)";
#define MHIQueueBuffer(handle, buffer, size) __MHIQueueBuffer(MHIBase, (handle), (buffer), (size))

APTR __MHIGetEmpty(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-48(a6)";
#define MHIGetEmpty(handle) __MHIGetEmpty(MHIBase, (handle))

UBYTE __MHIGetStatus(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-54(a6)";
#define MHIGetStatus(handle) __MHIGetStatus(MHIBase, (handle))

VOID __MHIPlay(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-60(a6)";
#define MHIPlay(handle) __MHIPlay(MHIBase, (handle))

VOID __MHIStop(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-66(a6)";
#define MHIStop(handle) __MHIStop(MHIBase, (handle))

VOID __MHIPause(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-72(a6)";
#define MHIPause(handle) __MHIPause(MHIBase, (handle))

ULONG __MHIQuery(__reg("a6") struct Library *, __reg("d1") ULONG query)="\tjsr\t-78(a6)";
#define MHIQuery(query) __MHIQuery(MHIBase, (query))

VOID __MHISetParam(__reg("a6") struct Library *, __reg("a3") APTR handle, __reg("d0") UWORD param, __reg("d1") ULONG value)="\tjsr\t-84(a6)";
#define MHISetParam(handle, param, value) __MHISetParam(MHIBase, (handle), (param), (value))

#endif /*  _VBCCINLINE_MHI_H  */
#endif /* __VBCC__ */

#define NUMBUFS  8
#define BUFSIZE  128*128

extern struct ExecBase *SysBase;
extern struct DOSLibrary *DOSBase;

struct Library *MHIBase;

static char Version[] = "\0$VER: MHIplay 1.001 (06.04.2025)\0";

ASM( VOID ) mPutChProc( REG( d0, UBYTE c ), REG( a3, UBYTE ** target )) {

  **target = c;
  ++( *target );
}

static BYTE PrintBuffer[ 1024 ];
static BPTR ShellOut;
VOID PrintfV34( STRPTR format, ... ) {

  APTR output = ( APTR ) &PrintBuffer;
  LONG length;

  RawDoFmt(
  format,
    /*
    /------+-----------------------> Cast to required APTR 
    |      |  /------+-------------> Nice math in LONGs to make it work 
    |      |  |      | /-----+-----> STRPTR, address of format, first argument
    |      |  |      | |     | /-+-> 4 bytes later, the next argument follows
    |      |  |      | |     | | |   */
    ( APTR ) (( LONG ) &format + 4 ),
    &mPutChProc,
    &output );
  length = ( LONG ) output - ( LONG ) PrintBuffer;
  Write( ShellOut, PrintBuffer, length );
}

int main( int argc, char *argv[] ) {

  STRPTR PC[ 5 ] = { "-", "\\", "|", "/" };
  ULONG Progress;
  BYTE MHISignal;
  ULONG MHIMask, Signals;
  APTR MHIHandle;
  BPTR InFile;
  APTR Buffer[ NUMBUFS ];
  ULONG CurrentLen;
  APTR CurrentBuffer;
  BOOL MemOk;
  BOOL Done;
  ULONG ReturnCode;
  long i;

  ShellOut = Output();

  ReturnCode = 0;
  if ( argc != 3 ) {

    PrintfV34( "Usage: MHIplay <driver> <file>\n" );
    return 5;
  }

  if ( MHIBase = OpenLibrary( argv[ 1 ], 0 )) {

    MHISignal = AllocSignal( -1 );
    if ( MHISignal != -1 ) {

      MHIMask = 1L << MHISignal;
      if ( MHIHandle = MHIAllocDecoder( FindTask( NULL ), MHIMask )) {

        PrintfV34( "\nDriver details:\n" );
        PrintfV34( "Name:    %s\n",
			       ( STRPTR ) MHIQuery( MHIQ_DECODER_NAME ));
        PrintfV34( "Version: %s\n",
			       ( STRPTR ) MHIQuery( MHIQ_DECODER_VERSION ));
        PrintfV34( "Author:  %s\n",
			       ( STRPTR ) MHIQuery( MHIQ_AUTHOR ));
        PrintfV34( "\n" );

        /************************/
        /* Allocate all buffers */
        /************************/
        MemOk = TRUE;
        for ( i = 0; i < NUMBUFS; ++i ) {

          Buffer[ i ] = AllocMem( BUFSIZE, MEMF_CLEAR );
          if ( !Buffer[ i ]) {

            MemOk=FALSE;
		      }
        }
        if ( MemOk ) {
          if ( InFile = Open( argv[ 2 ], MODE_OLDFILE )) {

            Done = FALSE;

            /**************/
            /* Preloading */
            /**************/
            PrintfV34( "Preloading.\n" );
            for ( i = 0; i < NUMBUFS; i++ ) {
              if ( CurrentLen = Read( InFile, Buffer[ i ], BUFSIZE )) {

                MHIQueueBuffer( MHIHandle, Buffer[ i ], CurrentLen );
                if ( CurrentLen != BUFSIZE ) {

                  Done=TRUE;
                }
              }
            }

            /******************/
            /* Playback start */
            /******************/
            PrintfV34( "Starting playback.\n" );
            MHIPlay( MHIHandle );
            Progress = 0;

            /*************/
            /* Main loop */
            /*************/
            while( !Done ) {

              PrintfV34( "Playing [%s]\r", PC[ Progress ]);

              ++Progress;
              if ( Progress > 3 ) {

                Progress = 0;
              }

              Signals = Wait( MHIMask | SIGBREAKF_CTRL_C );
              if ( Signals & SIGBREAKF_CTRL_C ) {

                break;
              }

              if ( Signals & MHIMask ) {

                /* Reload and queue all empty buffers */
                for ( i = 0; i < NUMBUFS; ++i ) {

                  if ( CurrentBuffer = MHIGetEmpty( MHIHandle )) {

                    if ( CurrentLen = Read( InFile, CurrentBuffer, BUFSIZE )) {

                      MHIQueueBuffer( MHIHandle, CurrentBuffer, CurrentLen );
                    }
                    if( CurrentLen != BUFSIZE ) {

                      Done=TRUE;
                    }
                  }
                }

                /* Restart if needed */
                if ( MHIGetStatus( MHIHandle ) == MHIF_OUT_OF_DATA ) {

                  MHIPlay(MHIHandle);
                }
              }
            }

            /*************************************/
            /* Wait for all buffers to run empty */
            /*************************************/
            if ( !( Signals & SIGBREAKF_CTRL_C )) {

              PrintfV34( "EOF reached. Waiting for end of stream.\n" );
              while (( MHIGetStatus( MHIHandle ) == MHIF_PLAYING )) {

                Signals = Wait( MHIMask | SIGBREAKF_CTRL_C );
                if ( Signals & SIGBREAKF_CTRL_C ) {

                  break;
                }				  
              }
            }

            /*****************/
            /* Playback stop */
            /*****************/
            PrintfV34( "Stopping playback.\n" );
            MHIStop( MHIHandle );
            Close( InFile );

          } else {

            PrintfV34( "Can't open file!\n" );
          }

        } else {

          PrintfV34( "Out of memory!\n" );
          ReturnCode = 5;
        }
        /********************/
        /* Free all buffers */
        /********************/
        for ( i = 0; i < NUMBUFS; ++i ) {

          if ( Buffer[ i ]) {

            FreeMem( Buffer[ i ], BUFSIZE );
          }
        }
        MHIFreeDecoder( MHIHandle );

      } else {

        PrintfV34( "Can't allocate decoder!\n" );
        ReturnCode = 5;
      }
      FreeSignal( MHISignal );

    } else {

      PrintfV34( "No signals available! Crazy man!\n" );
      ReturnCode = 5;
    }
    CloseLibrary( MHIBase );

  } else {

    PrintfV34( "Can't open MHI driver \"%s\"\n", argv[ 1 ] );
  }
  return ReturnCode;
}
