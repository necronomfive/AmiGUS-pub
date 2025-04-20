/*
 * This file is part of the mhiamigus.library.
 *
 * mhiamigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiamigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiamigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>

#include <proto/exec.h>

#include "amigus_hardware.h"
#include "amigus_mhi.h"
#include "interrupt.h"
#include "support.h"

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/

struct AmiGUS_MHI        * AmiGUS_MHI_Base   = NULL;
struct Device            * TimerBase         = NULL;

UWORD                    * ReadReg16queue    = NULL;
ULONG                      ReadPos16queue    = 0;
ULONG                    * ReadReg32queue    = NULL;
ULONG                      ReadPos32queue    = 0;
ULONG                    * WriteReg16queue   = NULL;
ULONG                      WritePos16queue   = 0;
ULONG                    * WriteReg32queue   = NULL;
ULONG                      WritePos32queue   = 0;

UWORD ReadReg16( APTR amiGUS, ULONG offset ) {

  UWORD result = ReadReg16queue[ ReadPos16queue ];
  printf( "Read16  #%lu = %lu = 0x%04lx for 0x%08lx\n",
          ReadPos16queue, ( ULONG ) result, ( ULONG ) result, offset );
  ++ReadPos16queue;
  return result;
}

ULONG ReadReg32( APTR amiGUS, ULONG offset ) {

  ULONG result = ReadReg32queue[ ReadPos32queue ];
  printf( "Read32  #%lu = %lu = 0x%08lx for 0x%08lx\n",
          ReadPos32queue, result, result, offset );
  ++ReadPos32queue;
  return result;
}

VOID WriteReg16( APTR amiGUS, ULONG offset, UWORD value ) {

  WriteReg16queue[ WritePos16queue ] = value;
  printf( "Write16 #%lu = %lu = 0x%08lx for 0x%08lx\n",
          WritePos16queue, ( ULONG ) value, ( ULONG ) value, offset );
  ++WritePos32queue;
}

VOID WriteReg32( APTR amiGUS, ULONG offset, ULONG value ) {

  WriteReg32queue[ WritePos32queue ] = value;
  printf( "Write32 #%lu = %lu = 0x%08lx for 0x%08lx\n",
          WritePos32queue, value, value, offset );
  ++WritePos32queue;
}

ULONG GetVS1063EndFill( APTR amiGUS ) {

  return 0xEEeeEEee;
}

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testAlignedPlayback( VOID ) {

  BOOL failed = FALSE;
  ULONG u;
  UWORD inBuffer16[] = {
    AMIGUS_PCM_REC_FIFO_WORDS - 1 - ( 2 << 1 ),
    AMIGUS_PCM_REC_FIFO_WORDS - 1 - ( 4 << 1 ),
    AMIGUS_PCM_REC_FIFO_WORDS - 1 - ( 2 << 1 ),
  };
  ULONG outBuffer32[ 10 ];

  struct AmiGUS_MHI_Buffer mhiBufferA;
  struct AmiGUS_MHI_Buffer mhiBufferB;
  ULONG bufferA[] = { 1, 2, 3, 4 };
  ULONG bufferB[] = { 5, 6, 7, 8 };
  struct AmiGUS_MHI_Handle * handle =
    ( struct AmiGUS_MHI_Handle * )
      AmiGUS_MHI_Base->agb_Clients.mlh_Head;

  mhiBufferA.agmb_Buffer = ( ULONG * ) &bufferA;
  mhiBufferA.agmb_BufferIndex = 0;
  mhiBufferA.agmb_BufferMax = 4;
  mhiBufferA.agmb_BufferExtraBytes = 0;

  mhiBufferB.agmb_Buffer = ( ULONG * ) &bufferB;
  mhiBufferB.agmb_BufferIndex = 0;
  mhiBufferB.agmb_BufferMax = 4;
  mhiBufferB.agmb_BufferExtraBytes = 0;

  NEW_LIST( &handle->agch_Buffers );
  AddTail( ( struct List * ) &handle->agch_Buffers, 
           ( struct Node * ) &mhiBufferA );
  AddTail( ( struct List * ) &handle->agch_Buffers,
           ( struct Node * ) &mhiBufferB );
  handle->agch_CurrentBuffer = &mhiBufferA;

  ReadReg16queue = inBuffer16;
  ReadPos16queue = 0;
  WriteReg32queue = &outBuffer32[ 1 ];
  WritePos32queue = 0;
  outBuffer32[ 0 ] = 0;
  outBuffer32[ 9 ] = 0;

  printf("Call 1:\n");
  HandlePlayback( handle );
  printf("Call 2:\n");
  HandlePlayback( handle );
  printf("Call 3:\n");
  HandlePlayback( handle );
  printf("Calls done.\n");

  for ( u = 0 ; u <= 8 ; ++u ) {

    failed |= ( outBuffer32[ u ] != u );
    // printf("u = %ld is %ld\n", u, failed);
  }
  failed |= ( outBuffer32[ 9 ] != 0 );
  printf( "\nAligned Playback test %s\n\n", failed ? "failed" : "OK" );

  return failed;
}

BOOL testUnalignedPlayback( VOID ) {

  BOOL failed = FALSE;
  ULONG u;
  UWORD inBuffer16[] = {
    AMIGUS_PCM_REC_FIFO_WORDS - 1 - ( 2 << 1 ),
    AMIGUS_PCM_REC_FIFO_WORDS - 1 - ( 4 << 1 ),
    AMIGUS_PCM_REC_FIFO_WORDS - 1 - ( 4 << 1 ),
  };
  ULONG outBuffer32[ 11 ];
  ULONG expectedBuffer32[ 11 ] = {
    0x00000000,
    0x00010002, 0x00030004, 0x00050006, 0x00070008,
    0x123456EE,
    0x00000005, 0x00000006, 0x00000007, 0x00000008,
    0x00000000
  };

  struct AmiGUS_MHI_Buffer mhiBufferA;
  struct AmiGUS_MHI_Buffer mhiBufferB;
  UBYTE bufferA[] = { 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8,
                      0x12, 0x34, 0x56, 0x78 };
  ULONG bufferB[] = { 5, 6, 7, 8 };
  struct AmiGUS_MHI_Handle * handle =
    ( struct AmiGUS_MHI_Handle * )
      AmiGUS_MHI_Base->agb_Clients.mlh_Head;

  mhiBufferA.agmb_Buffer = ( ULONG * ) &bufferA;
  mhiBufferA.agmb_BufferIndex = 0;
  mhiBufferA.agmb_BufferMax = 4;
  mhiBufferA.agmb_BufferExtraBytes = 3;

  mhiBufferB.agmb_Buffer = ( ULONG * ) &bufferB;
  mhiBufferB.agmb_BufferIndex = 0;
  mhiBufferB.agmb_BufferMax = 4;
  mhiBufferB.agmb_BufferExtraBytes = 0;

  NEW_LIST( &handle->agch_Buffers );
  AddTail( ( struct List * ) &handle->agch_Buffers, 
           ( struct Node * ) &mhiBufferA );
  AddTail( ( struct List * ) &handle->agch_Buffers,
           ( struct Node * ) &mhiBufferB );
  handle->agch_CurrentBuffer = &mhiBufferA;

  ReadReg16queue = inBuffer16;
  ReadPos16queue = 0;
  WriteReg32queue = &outBuffer32[ 1 ];
  WritePos32queue = 0;
  outBuffer32[ 0 ] = 0;
  outBuffer32[ 10 ] = 0;

  printf("Call 1:\n");
  HandlePlayback( handle );
  printf("Call 2:\n");
  HandlePlayback( handle );
  printf("Call 3:\n");
  HandlePlayback( handle );
  printf("Calls done.\n");

  for ( u = 0 ; u < 11 ; ++u ) {

    failed |= ( outBuffer32[ u ] != expectedBuffer32[ u ] );
    ///**/
    printf( "u = %ld is %ld - exp 0x%08lx act 0x%08lx\n",
            ( LONG ) u,
            ( LONG ) failed,
            expectedBuffer32[ u ],
            outBuffer32[ u ] );
   /*  */
  }
  printf( "\nUnaligned Playback test %s\n\n", failed ? "failed" : "OK" );
  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;

  struct AmiGUS_MHI_Handle * handle = 
    malloc( sizeof( struct AmiGUS_MHI_Handle ));
  AmiGUS_MHI_Base = malloc( sizeof( struct AmiGUS_MHI ));
  memset( AmiGUS_MHI_Base, 0, sizeof( struct AmiGUS_MHI ));
  NEW_LIST( &AmiGUS_MHI_Base->agb_Clients );
  AddTail(( struct List * ) &AmiGUS_MHI_Base->agb_Clients,
          ( struct Node * ) handle );

  if ( !AmiGUS_MHI_Base ) {

    printf( "Memory allocation failed!" );
    return 20;
  }

  failed |= testAlignedPlayback();
  failed |= testUnalignedPlayback();

  free( handle );
  free( AmiGUS_MHI_Base );

  return ( failed ) ? 15 : 0;
}
