#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "amigus_ahi_modes.h"
#include "amigus_ahi_sub.h"
#include "amigus_hardware.h"
#include "copies.h"

#if defined (__VBCC__)
/* Don't care about ugly type issues with format strings! */
#pragma dontwarn 214
#endif

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/
struct AmiGUSBase * AmiGUSBase;

/* Taken over from lib_amigus.c */
/*
 defined in LIB:c.o:
struct ExecBase          * SysBase           = 0;
struct DosLibrary        * DOSBase           = 0;
struct IntuitionBase     * IntuitionBase     = 0;
struct Library           * UtilityBase       = 0;
struct Library           * ExpansionBase     = 0;
 */
struct Device            * TimerBase         = 0;

char testFIFO[ 16 ][ 16 ];
ULONG * testLongFIFO = ( ULONG * )testFIFO;
ULONG nextTestFIFO = 0;

VOID flushFIFO( VOID ) {

  int i;
  for ( i = 0; 16 > i; ++i ) {

    testLongFIFO[ i ] = 0;
  }
  nextTestFIFO = 0;
}

/* From amigus_hardware.c */

const LONG AmiGUSSampleRates[ AMIGUS_PCM_SAMPLE_RATE_COUNT ] = {

   8000, // AMIGUS_PCM_SAMPLE_RATE_8000  @ index 0x0000
  11025, // AMIGUS_PCM_SAMPLE_RATE_11025 @ index 0x0001
  16000, // AMIGUS_PCM_SAMPLE_RATE_16000 @ index 0x0002
  22050, // AMIGUS_PCM_SAMPLE_RATE_22050 @ index 0x0003
  24000, // AMIGUS_PCM_SAMPLE_RATE_24000 @ index 0x0004
  32000, // AMIGUS_PCM_SAMPLE_RATE_32000 @ index 0x0005
  44100, // AMIGUS_PCM_SAMPLE_RATE_44100 @ index 0x0006
  48000  // AMIGUS_PCM_SAMPLE_RATE_48000 @ index 0x0007
//96000  // AMIGUS_PCM_SAMPLE_RATE_96000 @ index 0x0008
};

UWORD ReadReg16( APTR amiGUS, ULONG offset ) {

  ULONG result = testLongFIFO[ nextTestFIFO ];
  ++nextTestFIFO;
  return ( UWORD )( result & 0xffFF );
}

ULONG ReadReg32( APTR amiGUS, ULONG offset ) {

  ULONG result = testLongFIFO[ nextTestFIFO ];
  ++nextTestFIFO;
  return result;
}

VOID WriteReg32( APTR amiGUS, ULONG offset, ULONG value ) {

  sprintf( testFIFO[ nextTestFIFO], "%08lx", value );
  ++nextTestFIFO;
}

/******************************************************************************
 * Private functions / fields under test:
 *****************************************************************************/

UWORD gcd( UWORD a, UWORD b );
ULONG lcm( ULONG a, ULONG b );
extern const ULONG CopyFunctionRequirementById[];

/******************************************************************************
 * Test functions:
 *****************************************************************************/

/*
remainders from issues in calling the copy function caused by SAVEDS pragmas
LONG localPlainLongCopy1(
  ULONG *bufferBase, 
  ULONG *bufferIndex ) {

  ULONG addressIn = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG in = *(( ULONG * ) addressIn);
  ULONG out = in;

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, out );

  ( *bufferIndex ) += 1;
  return 4;
}

ASM(LONG) localPlainLongCopy2(
  REG(d0, ULONG *bufferBase ), 
  REG(a0, ULONG *bufferIndex ) ) {

  ULONG addressIn = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG in = *(( ULONG * ) addressIn);
  ULONG out = in;

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, out );

  ( *bufferIndex ) += 1;
  return 4;
}
*/

BOOL testPlaybackCopy16to8( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /********* for copy function tests, just adapt the between section *********/
  ULONG in[] = { 0x00000000, 0x12345678, 0x9abcdef0, 0xffFFffFF };
  ULONG index = 1;
  ULONG exp[] = { 3, 4, 1, 1 };
  STRPTR expF[] = { "12569ade" };

  AmiGUSBase->agb_Playback.agpp_CopyFunction = &PlaybackCopy16to8;
  printf("\nTesting PlaybackCopy16to8 ...\n");
  /********* for copy function tests, just adapt the between section *********/
  
  flushFIFO();

  out = (* AmiGUSBase->agb_Playback.agpp_CopyFunction)( in, &index );

  tst0 = ( exp[ 0 ] == index );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= !( strcmp( testFIFO[ i ], expF[ i ] ) );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "FIFO content:                                   %s - \t%s\n",
          exp[ 0 ], index, (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "FIFO LONG #%ld:\t   %s (expected) - %s (actual)\n",
            i, expF[ i ], testFIFO[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

  return failed;
}

BOOL testPlaybackCopy16to16( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /********* for copy function tests, just adapt the between section *********/
  ULONG in[] = { 0x00000000, 0x12345678, 0xffFFffFF };
  ULONG index = 1;
  ULONG exp[] = { 2, 4, 1, 1 };
  STRPTR expF[] = { "12345678" };

  AmiGUSBase->agb_Playback.agpp_CopyFunction = &PlaybackCopy16to16;
  printf("\nTesting PlaybackCopy16to16 ...\n");
  /********* for copy function tests, just adapt the between section *********/
  
  flushFIFO();

  out = (* AmiGUSBase->agb_Playback.agpp_CopyFunction)( in, &index );

  tst0 = ( exp[ 0 ] == index );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= !( strcmp( testFIFO[ i ], expF[ i ] ) );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "FIFO content:                                   %s - \t%s\n",
          exp[ 0 ], index, (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "FIFO LONG #%ld:\t   %s (expected) - %s (actual)\n",
            i, expF[ i ], testFIFO[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

  return failed;
}

BOOL testPlaybackCopy32to8( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /********* for copy function tests, just adapt the between section *********/
  ULONG in[] =  {
    0x00000000,
    0x12345678,
    0x9abcdef0,
    0x0fedcba9,
    0x87654321,
    0xffFFffFF };
  ULONG index = 1;
  ULONG exp[] = { 5, 4, 1, 1 };
  STRPTR expF[] = { "129a0f87" };

  AmiGUSBase->agb_Playback.agpp_CopyFunction = &PlaybackCopy32to8;
  printf("\nTesting PlaybackCopy32to8 ...\n");
  /********* for copy function tests, just adapt the between section *********/
  
  flushFIFO();

  out = (* AmiGUSBase->agb_Playback.agpp_CopyFunction)( in, &index );

  tst0 = ( exp[ 0 ] == index );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= !( strcmp( testFIFO[ i ], expF[ i ] ) );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "FIFO content:                                   %s - \t%s\n",
          exp[ 0 ], index, (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "FIFO LONG #%ld:\t   %s (expected) - %s (actual)\n",
            i, expF[ i ], testFIFO[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

  return failed;
}

BOOL testPlaybackCopy32to16( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /********* for copy function tests, just adapt the between section *********/
  ULONG in[] = { 0x00000000, 0x12345678, 0x9abcdef0, 0xffFFffFF };
  ULONG index = 1;
  ULONG exp[] = { 3, 4, 1, 1 };
  STRPTR expF[] = { "12349abc" };

  AmiGUSBase->agb_Playback.agpp_CopyFunction = &PlaybackCopy32to16;
  printf("\nTesting PlaybackCopy32to16 ...\n");
  /********* for copy function tests, just adapt the between section *********/
  
  flushFIFO();

  out = (* AmiGUSBase->agb_Playback.agpp_CopyFunction)( in, &index );

  tst0 = ( exp[ 0 ] == index );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= !( strcmp( testFIFO[ i ], expF[ i ] ) );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "FIFO content:                                   %s - \t%s\n",
          exp[ 0 ], index, (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "FIFO LONG #%ld:\t   %s (expected) - %s (actual)\n",
            i, expF[ i ], testFIFO[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

  return failed;
}

BOOL testPlaybackCopy32to24( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /********* for copy function tests, just adapt the between section *********/
  ULONG in[] = {
    0x00000000,
    0x12345678,
    0x9abcdef0,
    0x0fedcba9,
    0x87654321,
    0xffFFffFF };
  ULONG index = 1;
  ULONG exp[] = { 5, 12, 3, 3 };
  STRPTR expF[] = {
    "1234569a",
    "bcde0fed",
    "cb876543" };

  AmiGUSBase->agb_Playback.agpp_CopyFunction = &PlaybackCopy32to24;
  printf("\nTesting PlaybackCopy32to24 ...\n");
  /********* for copy function tests, just adapt the between section *********/
  
  flushFIFO();

  out = (* AmiGUSBase->agb_Playback.agpp_CopyFunction)( in, &index );

  tst0 = ( exp[ 0 ] == index );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= !( strcmp( testFIFO[ i ], expF[ i ] ) );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "FIFO content:                                   %s - \t%s\n",
          exp[ 0 ], index, (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "FIFO LONG #%ld:\t   %s (expected) - %s (actual)\n",
            i, expF[ i ], testFIFO[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

  return failed;
}

BOOL testPlaybackCopyFunctionCalling( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /********* for copy function tests, just adapt the between section *********/
  ULONG in[] = { 0x00000000, 0x12345678, 0xffFFffFF };
  ULONG exp[] = { 2, 4, 1, 1 };
  STRPTR expF[] = { "12345678" };

  AmiGUSBase->agb_Playback.agpp_CopyFunction =
    PlaybackPropertiesById[ 4 ].pp_CopyFunction;
  AmiGUSBase->agb_Playback.agpp_Buffer[ 0 ] = in;
  AmiGUSBase->agb_Playback.agpp_BufferIndex[ 0 ] = 1;
  printf("\nTesting PlaybackCopyFunctionCalling ...\n");
  /********* for copy function tests, just adapt the between section *********/
  
  flushFIFO();

  out = (* AmiGUSBase->agb_Playback.agpp_CopyFunction)(
    AmiGUSBase->agb_Playback.agpp_Buffer[ 0 ],
    &( AmiGUSBase->agb_Playback.agpp_BufferIndex[ 0 ] ));

  tst0 = ( exp[ 0 ] == AmiGUSBase->agb_Playback.agpp_BufferIndex[ 0 ] );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= !( strcmp( testFIFO[ i ], expF[ i ] ) );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "FIFO content:                                   %s - \t%s\n",
          exp[ 0 ], AmiGUSBase->agb_Playback.agpp_BufferIndex[ 0 ], (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "FIFO LONG #%ld:\t   %s (expected) - %s (actual)\n",
            i, expF[ i ], testFIFO[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

  return failed;
}

BOOL testRecordingCopy8Mto16S( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /**** for recording copy function tests, just adapt the between section ****/
  ULONG in[] = {
    0x12345678 };
  ULONG target[] = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000 };
  ULONG index = 1; // into target!
  ULONG exp[] = { 5, 16, 1, 6 };
  ULONG expF[] = {
    0x00000000,
    0x12001200,
    0x34003400,
    0x56005600,
    0x78007800,
    0x00000000 };

  AmiGUSBase->agb_Recording.agpr_CopyFunction = &RecordingCopy8Mto16S;
  printf("\nTesting RecordingCopy8Mto16S ...\n");
  /**** for recording copy function tests, just adapt the between section ****/

  flushFIFO();
  for ( i = 0; i < 1; ++i ) {

    testLongFIFO[ i ] = in[ i ];
  }

  out = (* AmiGUSBase->agb_Recording.agpr_CopyFunction)( target, &index );

  tst0 = ( exp[ 0 ] == index );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= ( target[ i ] == expF[ i ] );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "AHI Buffer content:                             %s - \t%s\n",
          exp[ 0 ], index, (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "AHI Buf. LONG #%ld:  %08lx (expected) - %08lx (actual)\n",
            i, expF[ i ], target[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

  return failed;
}

BOOL testRecordingCopy8Sto16S( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /**** for recording copy function tests, just adapt the between section ****/
  ULONG in[] = {
    0x12345678 };
  ULONG target[] = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000 };
  ULONG index = 1; // into target!
  ULONG exp[] = { 3, 8, 1, 4 };
  ULONG expF[] = {
    0x00000000,
    0x12003400,
    0x56007800,
    0x00000000 };

  AmiGUSBase->agb_Recording.agpr_CopyFunction = &RecordingCopy8Sto16S;
  printf("\nTesting RecordingCopy8Sto16S ...\n");
  /**** for recording copy function tests, just adapt the between section ****/

  flushFIFO();
  for ( i = 0; i < 1; ++i ) {

    testLongFIFO[ i ] = in[ i ];
  }

  out = (* AmiGUSBase->agb_Recording.agpr_CopyFunction)( target, &index );

  tst0 = ( exp[ 0 ] == index );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= ( target[ i ] == expF[ i ] );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "AHI Buffer content:                             %s - \t%s\n",
          exp[ 0 ], index, (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "AHI Buf. LONG #%ld:  %08lx (expected) - %08lx (actual)\n",
            i, expF[ i ], target[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

  return failed;
}

BOOL testRecordingCopy16Mto16S( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /**** for recording copy function tests, just adapt the between section ****/
  ULONG in[] = {
    0x12345678 }; // match line 824
  ULONG target[] = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000 };
  ULONG index = 1; // into target!
  ULONG exp[] = { 3, 8, 1, 4 };
  ULONG expF[] = {
    0x00000000,
    0x12341234,
    0x56785678,
    0x00000000 };

  AmiGUSBase->agb_Recording.agpr_CopyFunction = &RecordingCopy16Mto16S;
  printf("\nTesting RecordingCopy16Mto16S ...\n");
  /**** for recording copy function tests, just adapt the between section ****/

  flushFIFO();
  for ( i = 0; i < 1; ++i ) {

    testLongFIFO[ i ] = in[ i ];
  }

  out = (* AmiGUSBase->agb_Recording.agpr_CopyFunction)( target, &index );

  tst0 = ( exp[ 0 ] == index );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= ( target[ i ] == expF[ i ] );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "AHI Buffer content:                             %s - \t%s\n",
          exp[ 0 ], index, (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "AHI Buf. LONG #%ld:  %08lx (expected) - %08lx (actual)\n",
            i, expF[ i ], target[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

  return failed;
}

BOOL testRecordingCopy16Sto16S( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /**** for recording copy function tests, just adapt the between section ****/
  ULONG in[] = {
    0x12345678 };
  ULONG target[] = {
    0x00000000,
    0x00000000,
    0x00000000 };
  ULONG index = 1; // into target!
  ULONG exp[] = { 2, 4, 1, 3 };
  ULONG expF[] = {
    0x00000000,
    0x12345678,
    0x00000000 };

  AmiGUSBase->agb_Recording.agpr_CopyFunction = &RecordingCopy16Sto16S;
  printf("\nTesting RecordingCopy16Sto16S ...\n");
  /**** for recording copy function tests, just adapt the between section ****/

  flushFIFO();
  for ( i = 0; i < 1; ++i ) {

    testLongFIFO[ i ] = in[ i ];
  }

  out = (* AmiGUSBase->agb_Recording.agpr_CopyFunction)( target, &index );

  tst0 = ( exp[ 0 ] == index );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= ( target[ i ] == expF[ i ] );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "AHI Buffer content:                             %s - \t%s\n",
          exp[ 0 ], index, (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "AHI Buf. LONG #%ld:  %08lx (expected) - %08lx (actual)\n",
            i, expF[ i ], target[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

  return failed;
}

BOOL testRecordingCopy24Mto32S( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /**** for recording copy function tests, just adapt the between section ****/
  ULONG in[] = {
    0x12345678,
    0x00009abc }; // ReadReg16 just returns lower WORD!
  ULONG target[] = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000 };
  ULONG index = 1; // into target!
  ULONG exp[] = { 7, 24, 2, 8 };
  ULONG expF[] = {
    0x00000000,
    0x12340100,
    0x12340100,
    0x56780200,
    0x56780200,
    0x9abc0300,
    0x9abc0300,
    0x00000000 };

  AmiGUSBase->agb_Recording.agpr_CopyFunction = &RecordingCopy24Mto32S;
  printf("\nTesting RecordingCopy24Mto32S ...\n");
  /**** for recording copy function tests, just adapt the between section ****/

  flushFIFO();
  for ( i = 0; i < 2; ++i ) {

    testLongFIFO[ i ] = in[ i ];
  }

  out = (* AmiGUSBase->agb_Recording.agpr_CopyFunction)( target, &index );

  tst0 = ( exp[ 0 ] == index );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= ( target[ i ] == expF[ i ] );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "AHI Buffer content:                             %s - \t%s\n",
          exp[ 0 ], index, (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "AHI Buf. LONG #%ld:  %08lx (expected) - %08lx (actual)\n",
            i, expF[ i ], target[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

  return failed;
}

BOOL testRecordingCopy24Sto32S( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  /**** for recording copy function tests, just adapt the between section ****/
  ULONG in[] = {
    0x12345678,
    0x9abcdef0,
    0x0fedcba9,
    0x87654321 };
  ULONG target[] = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000 };
  ULONG index = 1; // into target!
  ULONG exp[] = { 7, 24, 3, 8 };
  ULONG expF[] = {
    0x00000000,
    0x12340100,
    0x56780200,
    0x9abc0300,
    0xdef00400,
    0x0fed0500,
    0xcba90600,
    0x00000000,
    0x87650700,
    0x43210800,
    0x00000000 };

  AmiGUSBase->agb_Recording.agpr_CopyFunction = &RecordingCopy24Sto32S;
  printf("\nTesting RecordingCopy24Sto32S ...\n");
  /**** for recording copy function tests, just adapt the between section ****/

  flushFIFO();
  for ( i = 0; i < 4; ++i ) {

    testLongFIFO[ i ] = in[ i ];
  }

  out = (* AmiGUSBase->agb_Recording.agpr_CopyFunction)( target, &index );

  tst0 = ( exp[ 0 ] == index );
  tst1 = ( exp[ 1 ] == out );
  tst2 = ( exp[ 2 ] == nextTestFIFO );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    tst3 &= ( target[ i ] == expF[ i ] );
  }

  printf( "Next buffer index: %8ld (expected) - %8ld (actual) - \t%s\n"
          "Bytes written:     %8ld (expected) - %8ld (actual) - \t%s\n"
          "Next FIFO index:   %8ld (expected) - %8ld (actual) - \t%s\n"
          "AHI Buffer content:                             %s - \t%s\n",
          exp[ 0 ], index, (tst0) ? "passed" : "FAIL!!",
          exp[ 1 ], out, (tst1) ? "passed" : "FAIL!!",
          exp[ 2 ], nextTestFIFO, (tst2) ? "passed" : "FAIL!!",
          "          ", (tst3) ? "passed" : "FAIL!!" );
  for ( i = 0; i < exp[ 3 ]; ++i ) {

    printf( "AHI Buf. LONG #%ld:  %08lx (expected) - %08lx (actual)\n",
            i, expF[ i ], target[ i ] );
  }

  failed |= !( tst0 & tst1 & tst2 & tst3 );

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

  failed |= testPlaybackCopy16to8();
  failed |= testPlaybackCopy16to16();
  failed |= testPlaybackCopy32to8();
  failed |= testPlaybackCopy32to16();
  failed |= testPlaybackCopy32to24();
  failed |= testPlaybackCopyFunctionCalling();
  failed |= testRecordingCopy8Mto16S();
  failed |= testRecordingCopy8Sto16S();
  failed |= testRecordingCopy16Mto16S();
  failed |= testRecordingCopy16Sto16S();
  failed |= testRecordingCopy24Mto32S();
  failed |= testRecordingCopy24Sto32S();

  free( AmiGUSBase );

  return ( failed ) ? 15 : 0;
}
