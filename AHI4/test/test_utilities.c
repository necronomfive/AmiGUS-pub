#include <stdio.h>

#include "amigus_private.h"
#include "utilities.h"

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/
struct AmiGUSBasePrivate * AmiGUSBase        = 0;

VOID WriteReg32( APTR amiGUS, ULONG offset, ULONG value ) {

  //sprintf( "0x%08lx", value );
}

UWORD gcd( UWORD a, UWORD b );
ULONG lcm( ULONG a, ULONG b );

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testGcd() {

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

BOOL testLcm() {

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

BOOL testGetBufferSizes() {

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

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;

  failed |= testGcd();
  failed |= testLcm();
  failed |= testGetBufferSizes();


  return ( failed ) ? 15 : 0;
}
