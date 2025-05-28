#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "amigus_ahi_modes.h"
#include "amigus_ahi_sub.h"
#include "amigus_hardware.h"
#include "buffers.h"

#if defined (__VBCC__)
/* Don't care about ugly type issues with format strings! */
#pragma dontwarn 214
#endif

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/
struct AmiGUS_AHI_Base * AmiGUS_AHI_Base;

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

  printf("ReadReg16-mock not implemented!\n");
  return 0;
}

ULONG ReadReg32( APTR amiGUS, ULONG offset ) {

  printf("ReadReg32-mock not implemented!\n");
  return 0;
}

VOID WriteReg32( APTR amiGUS, ULONG offset, ULONG value ) {

  printf("WriteReg32-mock not implemented!\n");
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

ULONG alignBufferSamplesRef( ULONG ahiBuffSamples ) {

  struct PlaybackProperties * mode = 
    &PlaybackPropertiesById[ AmiGUS_AHI_Base->agb_AhiModeOffset ];
  ULONG mask = mode->pp_AhiBufferMask;
  UBYTE shift = mode->pp_AhiSampleShift;
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
  UBYTE modeId;         /* 0 - 20 */
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

  for ( modeId = 0; 21 > modeId; ++modeId ) {
    printf( "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", 
            h0, h1, h1, h1, h2,
            h3,
            h4, h5, h5, h5, h6,
            h0, h1, h1, h1, h2 );
    for ( shift = 1; 4 > shift; ++shift ) {
      for ( i = 0; NUM_SAMPLE_RATES > i; ++i ) {

        suggestedBufferSize = sampleRates[ i ] / 100;
        AmiGUS_AHI_Base->agb_AhiModeOffset = modeId;

        alignedBufferSize = AlignByteSizeForSamples( suggestedBufferSize ) >>
          PlaybackPropertiesById[ modeId ].pp_AhiSampleShift;
        ref = alignBufferSamplesRef( suggestedBufferSize );

        exp0 = ( alignedBufferSize <= suggestedBufferSize );
        exp1 = ( suggestedBufferSize - alignedBufferSize <= 6 );
        exp2 = ( ref == alignedBufferSize );

        printf( "%3ld | %6ld | %5ld | %5ld | %5ld | %5ld | %5ld | %5ld |"
                " %5ld | %1ld | %6s\n",
                modeId, sampleRates[ i ], shift,
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

  modeId = AmiGUS_AHI_Base->agb_AhiModeOffset;
  printf( "Example mask: 0x%08lx, Example ~ mask: 0x%08lx\n",
          PlaybackPropertiesById[ modeId ].pp_AhiBufferMask,
          ~PlaybackPropertiesById[ modeId ].pp_AhiBufferMask );
  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;

  AmiGUS_AHI_Base = malloc( sizeof( struct AmiGUS_AHI_Base ) );
  memset( AmiGUS_AHI_Base, 0, sizeof( struct AmiGUS_AHI_Base ) );

  if ( !AmiGUS_AHI_Base ) {

    printf( "Memory allocation failed!" );
    return 20;
  }

  failed |= testGcd();
  failed |= testLcm();
  failed |= testGetBufferSizes();
//  failed |= testAlignBuffSamples();

  free( AmiGUS_AHI_Base );

  return ( failed ) ? 15 : 0;
}
