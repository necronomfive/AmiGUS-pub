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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>

#include "amigus_mhi.h"
#include "amigus_hardware.h"
#include "amigus_vs1063.h"

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/

struct AmiGUS_MHI        * AmiGUS_MHI_Base   = NULL;

WORD Equalizer[ 9 ];

/*
VOID UpdateVS1063Equalizer( APTR card, UWORD equalizerLevel, WORD value ) {

  WORD level = equalizerLevel - VS1063_CODEC_ADDRESS_EQ5_LEVEL1;
  level >>= 1;
/ *
  printf( "Setting 0x%04lx aka level%ld -> %ld\n",
          equalizerLevel, level, value );
* /
  Equalizer[ level ] = value;
}
*/
const WORD AmiGUSDefaultEqualizer[ 9 ] = {
  0, /* +/- 0dB */   125, /* Hz */
  0, /* +/- 0dB */   500, /* Hz */
  0, /* +/- 0dB */  2000, /* Hz */
  0, /* +/- 0dB */  8000, /* Hz */
  0  /* +/- 0dB */
};

const UBYTE AmiGUSVolumeMapping[ 104 ] = {
  0xFE /*  0% */, 
  72 /*  1% */, 66 /*  2% */, 60 /*  3% */, 54 /*  4% */, 51 /*   5% */,
  48 /*  6% */, 45 /*  7% */, 42 /*  8% */, 41 /*  9% */, 39 /*  10% */,
  37 /* 11% */, 36 /* 12% */, 34 /* 13% */, 33 /* 14% */, 32 /*  15% */,
  31 /* 16% */, 30 /* 17% */, 30 /* 18% */, 29 /* 19% */, 28 /*  20% */,
  27 /* 21% */, 26 /* 22% */, 26 /* 23% */, 25 /* 24% */, 24 /*  25% */,
  23 /* 26% */, 23 /* 27% */, 22 /* 28% */, 21 /* 29% */, 21 /*  30% */,
  20 /* 31% */, 20 /* 32% */, 19 /* 33% */, 19 /* 34% */, 18 /*  35% */,
  18 /* 36% */, 17 /* 37% */, 17 /* 38% */, 16 /* 39% */, 16 /*  40% */,
  15 /* 41% */, 15 /* 42% */, 15 /* 43% */, 14 /* 44% */, 14 /*  45% */,
  13 /* 46% */, 13 /* 47% */, 12 /* 48% */, 12 /* 49% */, 12 /*  50% */,
  11 /* 51% */, 11 /* 52% */, 11 /* 53% */, 10 /* 54% */, 10 /*  55% */,
  10 /* 56% */,  9 /* 57% */,  9 /* 58% */,  8 /* 59% */,  8 /*  60% */,
   8 /* 61% */,  7 /* 62% */,  7 /* 63% */,  7 /* 64% */,  7 /*  65% */,
   6 /* 66% */,  6 /* 67% */,  6 /* 68% */,  6 /* 69% */,  6 /*  70% */,
   5 /* 71% */,  5 /* 72% */,  5 /* 73% */,  5 /* 74% */,  5 /*  75% */,
   4 /* 76% */,  4 /* 77% */,  4 /* 78% */,  4 /* 79% */,  4 /*  80% */,
   3 /* 81% */,  3 /* 82% */,  3 /* 83% */,  3 /* 84% */,  3 /*  85% */,
   2 /* 86% */,  2 /* 87% */,  2 /* 88% */,  2 /* 89% */,  2 /*  90% */,
   1 /* 91% */,  1 /* 92% */,  1 /* 93% */,  1 /* 94% */,  1 /*  95% */,
   0 /* 96% */,  0 /* 97% */,  0 /* 98% */,  0 /* 99% */,  0 /* 100% */,
  99          , 99          , 99 // padding back to LONGs
};

ULONG ReadReg32( APTR card, ULONG offset ) { return 0; }
UWORD ReadCodecSPI( APTR card, UWORD SPIregister ) { return 0; }
UWORD ReadVS1063Mem( APTR amiGUS, UWORD address ) { return 0; }

VOID WriteReg16( APTR card, ULONG offset, UWORD value ) {}
VOID WriteReg32( APTR card, ULONG offset, ULONG value ) {}
VOID WriteCodecSPI( APTR card, UWORD SPIregister, UWORD SPIvalue ) {}

VOID InitEqualizerMock() {

  int i;

  for ( i = 0; i < 9; ++i ) {

    Equalizer[ i ] = i;
  }
}

VOID WriteVS1063Mem( APTR amiGUS, UWORD address, UWORD value ) {

  UWORD index = address - VS1063_CODEC_ADDRESS_EQ5_LEVEL1;
  Equalizer[ index ] = value;
}

VOID SleepCodecTicks( ULONG ticks ) {}

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL check( WORD * expected ) {

  int i;
  BOOL result = FALSE;
  WORD bandStart = 0;

  for ( i = 0; i < 9; ) {

    BOOL failed = FALSE;
    WORD expGain = expected[ i ];
    WORD actGain = Equalizer[ i++ ];
    WORD expEnd = expected[ i ];
    WORD actEnd = ( 9 > i ) ? Equalizer[ i++ ] : 0x7fFF;

    failed |= ( actGain != expGain );
    if ( 9 > i ) {

      failed |= ( actEnd != expEnd );
    }
    printf( "Equalizer band%ld (%5ld - %5ld) -> expected %3ld - is %3ld\t"
            "-\t%s\n",
            ( LONG ) (( i >> 1 ) - 1 ),
            ( LONG ) bandStart,
            ( LONG ) actEnd,
            ( LONG ) expGain,
            ( LONG ) actGain,
            ( failed ) ? "failed" : "OK" );
    bandStart = expEnd;
    result |= failed;
  }

  return result;
}

BOOL testDefaultState( VOID ) {

  WORD expected[ 9 ] = { 0, 125, 0, 500, 0, 2000, 0, 8000, 0 };

  InitEqualizerMock();

  InitVS1063Equalizer( NULL, TRUE, AmiGUSDefaultEqualizer );

  return check( expected );
}

BOOL testMaxGainAllMid( VOID ) {

  WORD expected[ 9 ] = { 32, 125, 32, 500, 32, 2000, 32, 8000, 32 };
  UBYTE in[ 11 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100 };

  InitVS1063Equalizer( NULL, TRUE, AmiGUSDefaultEqualizer );
  UpdateVS1063Equalizer( NULL, in );

  return check( expected );
}

BOOL testMinGainAllMid( VOID ) {

  WORD expected[ 9 ] = { -32, 125, -32, 500, -32, 2000, -32, 8000, -32 };
  UBYTE in[ 11 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  InitVS1063Equalizer( NULL, TRUE, AmiGUSDefaultEqualizer );
  UpdateVS1063Equalizer( NULL, in );

  return check( expected );
}

BOOL testNoGainRamp( VOID ) {

  WORD expected[ 9 ] = { -13, 125, -7, 500, 0, 2000, 6, 8000, 13 };
  UBYTE in[ 11 ] = { 25, 35, 35, 45, 45, 55, 55, 65, 65, 75, 50 };

  InitVS1063Equalizer( NULL, TRUE, AmiGUSDefaultEqualizer );
  UpdateVS1063Equalizer( NULL, in );

  return check( expected );
}

BOOL testMaxGainRamp( VOID ) {

  WORD expected[ 9 ] = { 3, 125, 9, 500, 16, 2000, 22, 8000, 29 };
  UBYTE in[ 11 ] = { 25, 35, 35, 45, 45, 55, 55, 65, 65, 75, 100 };

  InitVS1063Equalizer( NULL, TRUE, AmiGUSDefaultEqualizer );
  UpdateVS1063Equalizer( NULL, in );

  return check( expected );
}

BOOL testMinGainRamp( VOID ) {

  WORD expected[ 9 ] = { -29, 125, -23, 500, -16, 2000, -10, 8000, -3 };
  UBYTE in[ 11 ] = { 25, 35, 35, 45, 45, 55, 55, 65, 65, 75, 0 };

  InitVS1063Equalizer( NULL, TRUE, AmiGUSDefaultEqualizer );
  UpdateVS1063Equalizer( NULL, in );

  return check( expected );
}

BOOL testSomeGainRamp( VOID ) {

  WORD expected[ 9 ] = { -5, 125, 1, 500, 8, 2000, 14, 8000, 21 };
  UBYTE in[ 11 ] = { 25, 35, 35, 45, 45, 55, 55, 65, 65, 75, 75 };

  InitVS1063Equalizer( NULL, TRUE, AmiGUSDefaultEqualizer );
  UpdateVS1063Equalizer( NULL, in );

  return check( expected );
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;

  AmiGUS_MHI_Base = malloc( sizeof( struct AmiGUS_MHI ));
  memset( AmiGUS_MHI_Base, 0, sizeof( struct AmiGUS_MHI ));
  memset( Equalizer, 255, sizeof( Equalizer ));

  if ( !AmiGUS_MHI_Base ) {

    printf( "Memory allocation failed!" );
    return 20;
  }

  failed |= testDefaultState();
  failed |= testMaxGainAllMid();
  failed |= testMinGainAllMid();
  failed |= testNoGainRamp();
  failed |= testMaxGainRamp();
  failed |= testMinGainRamp();
  failed |= testSomeGainRamp();

  free( AmiGUS_MHI_Base );

  return ( failed ) ? 15 : 0;
}
  