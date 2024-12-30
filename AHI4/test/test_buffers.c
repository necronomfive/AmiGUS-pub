#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "amigus_private.h"
#include "buffers.h"

#if defined (__VBCC__)
/* Don't care about ugly type issues with format strings! */
#pragma dontwarn 214
#endif

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/
struct AmiGUSBasePrivate * AmiGUSBase;

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
ULONG nextTestFIFO = 0;

VOID flushFIFO( VOID ) {

  int i;
  for ( i = 0; 16 > i; ++i ) {

    testFIFO[ i ][ 0 ] = 0;
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
  48000, // AMIGUS_PCM_SAMPLE_RATE_48000 @ index 0x0007
  96000  // AMIGUS_PCM_SAMPLE_RATE_96000 @ index 0x0008
};

VOID WriteReg32( APTR amiGUS, ULONG offset, ULONG value ) {

  sprintf( testFIFO[nextTestFIFO], "%08lx", value );
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

BOOL testGcd( VOID ) {

  BOOL failed = FALSE;
  int i;

#define NUM_CASES 4
  int cases[NUM_CASES][ 3 ] = {
    { 8, 12, 4},
    {12,  8, 4},
    { 2,  4, 2},
    { 8,  4, 4}
  };

  printf("\nTesting gcd - greatest common denominator ...\n");
  for( i = 0; i < NUM_CASES; ++i ) {

    int a = cases[i][0];
    int b = cases[i][1];
    int exp = cases[i][2];
    int act = gcd( a, b );
    BOOL tst = ( exp == act );
    printf(
       "gcd( %ld, %ld ) = %ld (expected) = %ld (actual) \t %s\n",
       a,
       b,
       exp,
       act,
       (tst) ? "passed" : "FAIL!!" );
    failed |= !( tst );
  }
  printf("\n");
  #undef NUM_CASES

  return failed;
}

BOOL testLcm( VOID ) {

  BOOL failed = FALSE;
  int i;

  #define NUM_CASES 6
  int cases[NUM_CASES][ 3 ] = {
    { 8, 12, 24},
    {12,  8, 24},
    { 3,  4, 12},
    { 4,  3, 12},
    { 2,  4, 4},
    { 4,  2, 4}
  };

  printf("\nTesting lcm - least common multiple ...\n");
  for( i = 0; i < NUM_CASES; ++i ) {

    int a = cases[i][0];
    int b = cases[i][1];
    int exp = cases[i][2];
    int act = lcm( a, b );
    BOOL tst = ( exp == act );
    printf(
       "lcm( %ld, %ld ) = %ld (expected) = %ld (actual) \t %s\n",
       a,
       b,
       exp,
       act,
       (tst) ? "passed" : "FAIL!!");
    failed |= !( tst );
  }
  printf("\n");
  #undef NUM_CASES

  return failed;
}

BOOL testGetBufferSizes( VOID ) {

  BOOL failed = FALSE;
  #define NUM_SAMPLE_RATES 9
  const LONG sampleRates[ NUM_SAMPLE_RATES ] = {

     8000, // AMIGUS_SAMPLE_RATE_8000  @ index 0x0000
    11025, // AMIGUS_SAMPLE_RATE_11025 @ index 0x0001
    16000, // AMIGUS_SAMPLE_RATE_16000 @ index 0x0002
    22050, // AMIGUS_SAMPLE_RATE_22050 @ index 0x0003
    24000, // AMIGUS_SAMPLE_RATE_24000 @ index 0x0004
    32000, // AMIGUS_SAMPLE_RATE_32000 @ index 0x0005
    44100, // AMIGUS_SAMPLE_RATE_44100 @ index 0x0006
    48000, // AMIGUS_SAMPLE_RATE_48000 @ index 0x0007
    96000  // AMIGUS_SAMPLE_RATE_96000 @ index 0x0008
  };
  #define NUM_SAMPLE_SIZES 3
  const UBYTE sampleSizes[ NUM_SAMPLE_SIZES ] = { 1, 2, 4 };
  #define NUM_MULTIPLES 3
  const UBYTE multiples[ NUM_MULTIPLES ] = { 4, 8, 16 };
  #define NUM_FLAGS 2
  const UBYTE flags[ NUM_FLAGS ] = { FALSE, TRUE };

  int i;
  int j;
  int k;
  int l;
  int m;

  printf("\nTesting buffer size calculation in bytes and AHI samples ...\n");
  for ( j = 0; j < NUM_SAMPLE_SIZES; ++j ) {
    for ( k = 0; k < NUM_MULTIPLES; ++k ) {
      for ( l = 0; l < NUM_FLAGS; ++l ) {
        for ( m = 0; m < NUM_FLAGS; ++m ) {
          for ( i = 0; i < NUM_SAMPLE_RATES; ++i ) {

            LONG a = sampleRates[ i ];
            UBYTE b = sampleSizes[ j ];
            UBYTE c = multiples[ k ];
            UBYTE d = flags[ l ];
            UBYTE e = flags[ m ];
            UWORD r = getBufferBytes( a, b, c, d, e );
            UWORD s = getBufferSamples( r, b, d );
            BOOL tst = !( r % c ) && ( s <= ( a / ( e ? 100 : 25 ) )  );
            printf(
                "rate %5ld size %2ld mult's %2ld stereo %ld rtime %ld => "
                "%5ld bytes = %4ld samples \t "
                "%s\n",
                a, b, c, d, e, 
                r, s,
               (tst) ? "passed" : "FAIL!!" );
            failed |= !( tst );
          } 
        } 
      }
    }
  }
  return failed;
}

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

BOOL testCopy16to8( VOID ) {

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

  AmiGUSBase->agb_Playback.agpp_CopyFunction = &Copy16to8;
  printf("\nTesting Copy16to8 ...\n");
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

BOOL testCopy16to16( VOID ) {

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

  AmiGUSBase->agb_Playback.agpp_CopyFunction = &Copy16to16;
  printf("\nTesting Copy16to16 ...\n");
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

BOOL testCopy32to8( VOID ) {

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

  AmiGUSBase->agb_Playback.agpp_CopyFunction = &Copy32to8;
  printf("\nTesting Copy32to8 ...\n");
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

BOOL testCopy32to16( VOID ) {

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

  AmiGUSBase->agb_Playback.agpp_CopyFunction = &Copy32to16;
  printf("\nTesting Copy32to16 ...\n");
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

BOOL testCopy32to24( VOID ) {

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

  AmiGUSBase->agb_Playback.agpp_CopyFunction = &Copy32to24;
  printf("\nTesting Copy32to24 ...\n");
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

BOOL testCopyFunctionCalling( VOID ) {

  UBYTE tst0 = FALSE;
  UBYTE tst1 = FALSE;
  UBYTE tst2 = FALSE;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out = 0;
  int i;

  /********* for copy function tests, just adapt the between section *********/
  ULONG in[] = { 0x00000000, 0x12345678, 0xffFFffFF };
  ULONG exp[] = { 2, 4, 1, 1 };
  STRPTR expF[] = { "12345678" };

  AmiGUSBase->agb_Playback.agpp_CopyFunction = &Copy16to16;
  AmiGUSBase->agb_Playback.agpp_Buffer[ 0 ] = in;
  AmiGUSBase->agb_Playback.agpp_BufferIndex[ 0 ] = 1;
  printf("\nTesting CopyFunctionCalling ...\n");
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

ULONG alignBufferSamplesRef( ULONG ahiBuffSamples ) {

  ULONG mask = CopyFunctionRequirementById[ AmiGUSBase->agb_Playback.agpp_CopyFunctionId ];
  UBYTE shift = AmiGUSBase->agb_AhiSampleShift;
  ULONG aligned = ahiBuffSamples;

  aligned <<= shift;
  aligned &= mask;
  aligned >>= shift;

  return aligned;
}

BOOL testAlignBuffSamples( VOID ) {

  #define NUM_SAMPLE_RATES 9
  const LONG sampleRates[ NUM_SAMPLE_RATES ] = {

     8000, // AMIGUS_SAMPLE_RATE_8000  @ index 0x0000
    11025, // AMIGUS_SAMPLE_RATE_11025 @ index 0x0001
    16000, // AMIGUS_SAMPLE_RATE_16000 @ index 0x0002
    22050, // AMIGUS_SAMPLE_RATE_22050 @ index 0x0003
    24000, // AMIGUS_SAMPLE_RATE_24000 @ index 0x0004
    32000, // AMIGUS_SAMPLE_RATE_32000 @ index 0x0005
    44100, // AMIGUS_SAMPLE_RATE_44100 @ index 0x0006
    48000, // AMIGUS_SAMPLE_RATE_48000 @ index 0x0007
    96000  // AMIGUS_SAMPLE_RATE_96000 @ index 0x0008
  };
  LONG shift;          /* 1 - 3 */
  LONG copyFunctionId; /* 0 - 4 */
  LONG suggestedBufferSize;
  LONG alignedBufferSize;
  LONG i;
  BOOL failed = FALSE;
  LONG ref;
  BOOL exp0;
  BOOL exp1;
  BOOL exp2;

  const char * h0 = "----+--------+-------+";
  const char * h1 = "-------+-------+";
  const char * h2 = "---+--------\n";
  const char * h3 = "    | sample | BYTE  |      IN       |      OUT      |"
                    "       MINUS       |\n";
  const char * h4 = " ID |  rate  | shift |";
  const char * h5 = " Smpls | BYTEs |";
  const char * h6 = " % | result\n";

  for ( copyFunctionId = 0; 5 > copyFunctionId; ++copyFunctionId ) {
    printf( "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
            h0, h1, h1, h1, h2,
            h3,
            h4, h5, h5, h5, h6,
            h0, h1, h1, h1, h2 );
    for ( shift = 1; 4 > shift; ++shift ) {
      for ( i = 0; NUM_SAMPLE_RATES > i; ++i ) {

        suggestedBufferSize = sampleRates[ i ] / 100;
        AmiGUSBase->agb_Playback.agpp_CopyFunctionId = copyFunctionId;
        AmiGUSBase->agb_AhiSampleShift = shift;

        alignedBufferSize = AlignByteSizeForSamples( suggestedBufferSize )
                            >> AmiGUSBase->agb_AhiSampleShift;
        ref = alignBufferSamplesRef( suggestedBufferSize );

        exp0 = ( alignedBufferSize <= suggestedBufferSize );
        exp1 = ( suggestedBufferSize - alignedBufferSize <= 6 );
        exp2 = ( ref == alignedBufferSize );

        printf( "%3ld | %6ld | %5ld | %5ld | %5ld | %5ld | %5ld | %5ld |"
                " %5ld | %1ld | %6s\n",
                copyFunctionId, sampleRates[ i ], shift,
                suggestedBufferSize, suggestedBufferSize << shift,
                alignedBufferSize, alignedBufferSize << shift,
                suggestedBufferSize - alignedBufferSize,
                (suggestedBufferSize - alignedBufferSize) << shift,
                (LONG)(
                  ((suggestedBufferSize - alignedBufferSize) << shift) * 100)
                  / (suggestedBufferSize << shift),
                ( exp0 && exp1 && exp2 ) ? "passed" : "FAIL!!" );

        failed |= !( exp0 && exp1 && exp2 );
      }
    }
  }
  printf( "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
          h0, h1, h1, h1, h2,
          h3,
          h4, h5, h5, h5, h6,
          h0, h1, h1, h1, h2 );
  printf( "Example mask: 0x%08lx, Example ~ mask: 0x%08lx\n",
          CopyFunctionRequirementById[ AmiGUSBase->agb_Playback.agpp_CopyFunctionId ],
          ~CopyFunctionRequirementById[ AmiGUSBase->agb_Playback.agpp_CopyFunctionId ] );
  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;

  AmiGUSBase = malloc( sizeof( struct AmiGUSBasePrivate ) );
  memset( AmiGUSBase, 0, sizeof( struct AmiGUSBasePrivate ) );

  if ( !AmiGUSBase ) {

    printf( "Memory allocation failed!" );
    return 20;
  }

  failed |= testGcd();
  failed |= testLcm();
  failed |= testGetBufferSizes();
  failed |= testCopy16to8();
  failed |= testCopy16to16();
  failed |= testCopy32to8();
  failed |= testCopy32to16();
  failed |= testCopy32to24();
  failed |= testCopyFunctionCalling();
  failed |= testAlignBuffSamples();

  free( AmiGUSBase );

  return ( failed ) ? 15 : 0;
}
