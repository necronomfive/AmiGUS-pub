#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "amigus_private.h"
#include "utilities.h"

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/
struct AmiGUSBasePrivate * AmiGUSBase;

char testFIFO[ 16 ][ 16 ];
ULONG nextTestFIFO = 0;

VOID flushFIFO( VOID ) {

  int i;
  for ( i = 0; 16 > i; ++i ) {

    testFIFO[ i ][ 0 ] = 0;
  }
  nextTestFIFO = 0;
}

VOID WriteReg32( APTR amiGUS, ULONG offset, ULONG value ) {

  sprintf( testFIFO[nextTestFIFO], "%08lx", value );
  ++nextTestFIFO;
}

UWORD gcd( UWORD a, UWORD b );
ULONG lcm( ULONG a, ULONG b );

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

BOOL copyFunctionTest( ULONG *in, ULONG index, ULONG *exp, STRPTR *expF ) {
  
}

BOOL testPlainLongCopy( VOID ) {

  UBYTE tst0;
  UBYTE tst1;
  UBYTE tst2;
  UBYTE tst3 = TRUE;
  BOOL failed = FALSE;
  LONG out;
  int i;

  ULONG in[] = { 0x00000000, 0x12345678, 0xffFFffFF };
  ULONG index = 1;
  ULONG exp[] = { 2, 4, 1, 1 };
  STRPTR expF[] = { "12345678" };

  AmiGUSBase->agb_CopyFunction = &PlainLongCopy;

  flushFIFO();

  out = (* AmiGUSBase->agb_CopyFunction)( in, &index );

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

BOOL testShift16LongCopy( VOID ) {

  BOOL failed = FALSE;

  flushFIFO();

  return failed;
}

BOOL testMerge24LongCopy( VOID ) {

  BOOL failed = FALSE;

  flushFIFO();

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

/*
  failed |= testGcd();
  failed |= testLcm();
  failed |= testGetBufferSizes();
*/
  failed |= testPlainLongCopy();
/*
  failed |= testShift16LongCopy();
  failed |= testMerge24LongCopy();
*/

  free( AmiGUSBase );

  return ( failed ) ? 15 : 0;
}
