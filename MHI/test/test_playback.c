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

struct AmiGUSBase        * AmiGUSBase        = NULL;
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
  printf( "Read16  #%ld = %ld for 0x%08lx\n", ReadPos16queue, result, offset );
  ++ReadPos16queue;
  return result;
}

ULONG ReadReg32( APTR amiGUS, ULONG offset ) {

  UWORD result = ReadReg32queue[ ReadPos32queue ];
  printf( "Read32  #%ld = %ld for 0x%08lx\n", ReadPos32queue, result, offset );
  ++ReadPos32queue;
  return result;
}

VOID WriteReg16( APTR amiGUS, ULONG offset, UWORD value ) {

  WriteReg16queue[ WritePos16queue ] = value;
  printf( "Write16 #%ld = %ld for 0x%08lx\n", WritePos16queue, value, offset );
  ++WritePos32queue;
}

VOID WriteReg32( APTR amiGUS, ULONG offset, ULONG value ) {

  WriteReg32queue[ WritePos32queue ] = value;
  printf( "Write32 #%ld = %ld for 0x%08lx\n", WritePos32queue, value, offset );
  ++WritePos32queue;
}

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testAlignedPlayback( VOID ) {

  BOOL failed = FALSE;
  ULONG u;
  UWORD inBuffer16[] = {
    AMIGUS_PCM_REC_FIFO_WORDS - ( 2 << 1 ),
    AMIGUS_PCM_REC_FIFO_WORDS - ( 4 << 1 ),
    AMIGUS_PCM_REC_FIFO_WORDS - ( 2 << 1 ),
  };
  ULONG outBuffer32[ 10 ];

  struct AmiGUSMhiBuffer mhiBufferA;
  struct AmiGUSMhiBuffer mhiBufferB;
  ULONG bufferA[] = { 1, 2, 3, 4 };
  ULONG bufferB[] = { 5, 6, 7, 8 };
  struct AmiGUSClientHandle * handle = &AmiGUSBase->agb_ClientHandle;

  mhiBufferA.agmb_Buffer = ( ULONG * ) &bufferA;
  mhiBufferA.agmb_BufferIndex = 0;
  mhiBufferA.agmb_BufferMax = 4;
  mhiBufferA.agmb_BufferExtraBytes = 0;

  mhiBufferB.agmb_Buffer = ( ULONG * ) &bufferB;
  mhiBufferB.agmb_BufferIndex = 0;
  mhiBufferB.agmb_BufferMax = 4;
  mhiBufferB.agmb_BufferExtraBytes = 0;

  NonConflictingNewMinList( &handle->agch_Buffers );
  AddTail( ( struct List * ) &handle->agch_Buffers, 
           ( struct Node * ) &mhiBufferA );
  AddTail( ( struct List * ) &handle->agch_Buffers,
           ( struct Node * ) &mhiBufferB );
  handle->agch_CurrentBuffer = &mhiBufferA;

  ReadReg16queue = inBuffer16;
  ReadPos32queue = 0;
  WriteReg32queue = &outBuffer32[ 1 ];
  WritePos32queue = 0;
  outBuffer32[ 0 ] = 0;
  outBuffer32[ 9 ] = 0;

  HandlePlayback();
  HandlePlayback();
  HandlePlayback();

  for ( u = 0 ; u <= 8 ; ++u ) {

    failed |= ( outBuffer32[ u ] != u );
  }
  failed |= ( outBuffer32[ 9 ] != 0 );
  printf( "\NAligned Playback test %s\n", failed ? "failed" : "OK" );

  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;

  AmiGUSBase = malloc( sizeof( struct AmiGUSBase ) );
  memset( AmiGUSBase, 0, sizeof( struct AmiGUSBase ) );

  if ( !AmiGUSBase ) {

    printf( "Memory allocation failed!" );
    return 20;
  }

  failed |= testAlignedPlayback();

  free( AmiGUSBase );

  return ( failed ) ? 15 : 0;
}