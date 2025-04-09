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

VOID UpdateEqualizer( UBYTE index, UBYTE percent );

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/

struct AmiGUS_MHI        * AmiGUS_MHI_Base   = NULL;

WORD Equalizer[ 5 ];

VOID UpdateVS1063Equalizer( APTR card, UWORD equalizerLevel, WORD value ) {

  WORD level = equalizerLevel - VS1063_CODEC_ADDRESS_EQ5_LEVEL1;
  level >>= 1;
/*
  printf( "Setting 0x%04lx aka level%ld -> %ld\n",
          equalizerLevel, level, value );
*/
  Equalizer[ level ] = value;
}

const WORD AmiGUSDefaultEqualizer[ 9 ] = {
  0, /* +/- 0dB */   125, /* Hz */
  0, /* +/- 0dB */   500, /* Hz */
  0, /* +/- 0dB */  2000, /* Hz */
  0, /* +/- 0dB */  8000, /* Hz */
  0  /* +/- 0dB */
};


VOID StartAmiGusCodecPlayback( VOID ) {

  /* Just a mock - :) */
}
  
VOID StopAmiGusCodecPlayback( VOID ) {

  /* Just a mock - :) */
}

BOOL CreateInterruptHandler( VOID ) {

  return FALSE;
}

VOID DestroyInterruptHandler( VOID ) {

  /* Just a mock - :) */
}

VOID InitVS1063Equalizer( APTR card, BOOL enable, const WORD * settings ) {

  /* Just a mock - :) */
}

VOID InitVS1063Codec( APTR card ) {

  /* Just a mock - :) */
}

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL check( WORD * expected ) {

  int i;
  BOOL result = FALSE;

  for ( i = 0; i < 5; ++i ) {

    BOOL failed = ( expected[ i ] != Equalizer[ i ] );
    printf( "Equalizer band%ld -> expected %3ld - is %3ld\t-\t%s\n",
            i, expected[i], Equalizer[ i ], failed ? "failed" : "OK" );
    result |= failed;
  }

  return result;
}

BOOL testDefaultState( VOID ) {

  WORD expected[ 5 ] = { 0, 0, 0, 0, 0 };
  int i;

  printf( "Testing all mid:\n" );
  for ( i = 0; i < 11; ++i ) {

    UpdateEqualizer( i, 50 );
  }

  return check( expected );
}

BOOL testGainState1( VOID ) {

  WORD expected[ 5 ] = { 32, 32, 32, 32, 32 };
  int i;

  printf( "Testing all mid, gain max\n" );
  for ( i = 0; i < 10; ++i ) {

    UpdateEqualizer( i, 50 );
  }
  UpdateEqualizer( 10, 100 );

  return check( expected );
}

BOOL testGainState2( VOID ) {

  WORD expected[ 5 ] = { -32, -32, -32, -32, -32 };
  int i;

  printf( "Testing all mid, gain min\n" );
  for ( i = 0; i < 10; ++i ) {

    UpdateEqualizer( i, 50 );
  }
  UpdateEqualizer( 10, 0 );

  return check( expected );
}

BOOL testGainState3( VOID ) {

  WORD expected[ 5 ] = { 32, 32, 32, 32, 32 };
  int i;

  printf( "Testing all min, gain max\n" );
  for ( i = 0; i < 10; ++i ) {

    UpdateEqualizer( i, 0 );
  }
  UpdateEqualizer( 10, 100 );

  return check( expected );
}

BOOL testGainState4( VOID ) {

  WORD expected[ 5 ] = { -32, -32, -32, -32, -32 };
  int i;

  printf( "Testing all max, gain min\n" );
  for ( i = 0; i < 10; ++i ) {

    UpdateEqualizer( i, 100 );
  }
  UpdateEqualizer( 10, 0 );

  return check( expected );
}

BOOL testGainState5( VOID ) {

  WORD expected[ 5 ] = { -32, 32, 0, 0, 0 };
  int i;

  printf( "Testing 0: min, 1: max, gain: min\n" );
  for ( i = 4; i < 10; ++i ) {

    UpdateEqualizer( i, 50 );
  }
  UpdateEqualizer( 0, 0 );
  UpdateEqualizer( 1, 0 );
  UpdateEqualizer( 2, 100 );
  UpdateEqualizer( 3, 100 );
  UpdateEqualizer( 10, 0 );

  return check( expected );
}

BOOL testGainState6( VOID ) {

  WORD expected[ 5 ] = { -32, 32, 0, 0, 0 };
  int i;

  printf( "Testing 0: min, 1: max, gain: max\n" );
  for ( i = 4; i < 10; ++i ) {

    UpdateEqualizer( i, 50 );
  }
  UpdateEqualizer( 0, 0 );
  UpdateEqualizer( 1, 0 );
  UpdateEqualizer( 2, 100 );
  UpdateEqualizer( 3, 100 );
  UpdateEqualizer( 10, 100 );

  return check( expected );
}

BOOL testDistState1( VOID ) {

  WORD expected[ 5 ] = { -13, -7, 0, 6, 13 };

  printf( "Testing 0-5: asc ramp gain: none\n" );
  UpdateEqualizer( 0, 30 );
  UpdateEqualizer( 1, 30 );
  UpdateEqualizer( 2, 40 );
  UpdateEqualizer( 3, 40 );
  UpdateEqualizer( 4, 50 );
  UpdateEqualizer( 5, 50 );
  UpdateEqualizer( 6, 60 );
  UpdateEqualizer( 7, 60 );
  UpdateEqualizer( 8, 70 );
  UpdateEqualizer( 9, 70 );
  UpdateEqualizer( 10, 50 );

  return check( expected );
}

BOOL testDistState2( VOID ) {

  WORD expected[ 5 ] = { -7, -1, 6, 12, 19 };

  printf( "Testing 0-5: asc ramp gain: some\n" );
  UpdateEqualizer( 0, 30 );
  UpdateEqualizer( 1, 30 );
  UpdateEqualizer( 2, 40 );
  UpdateEqualizer( 3, 40 );
  UpdateEqualizer( 4, 50 );
  UpdateEqualizer( 5, 50 );
  UpdateEqualizer( 6, 60 );
  UpdateEqualizer( 7, 60 );
  UpdateEqualizer( 8, 70 );
  UpdateEqualizer( 9, 70 );
  UpdateEqualizer( 10, 67 );

  return check( expected );
}

BOOL testDistState3( VOID ) {

  WORD expected[ 5 ] = { -20, -14, -7, -1, 6 };

  printf( "Testing 0-5: asc ramp gain: neg\n" );
  UpdateEqualizer( 0, 30 );
  UpdateEqualizer( 1, 30 );
  UpdateEqualizer( 2, 40 );
  UpdateEqualizer( 3, 40 );
  UpdateEqualizer( 4, 50 );
  UpdateEqualizer( 5, 50 );
  UpdateEqualizer( 6, 60 );
  UpdateEqualizer( 7, 60 );
  UpdateEqualizer( 8, 70 );
  UpdateEqualizer( 9, 70 );
  UpdateEqualizer( 10, 30 );

  return check( expected );
}

BOOL testDistState4( VOID ) {

  WORD expected[ 5 ] = { -32, -26, -19, -13, -6 };

  printf( "Testing 0-5: asc ramp gain: min\n" );
  UpdateEqualizer( 0, 30 );
  UpdateEqualizer( 1, 30 );
  UpdateEqualizer( 2, 40 );
  UpdateEqualizer( 3, 40 );
  UpdateEqualizer( 4, 50 );
  UpdateEqualizer( 5, 50 );
  UpdateEqualizer( 6, 60 );
  UpdateEqualizer( 7, 60 );
  UpdateEqualizer( 8, 70 );
  UpdateEqualizer( 9, 70 );
  UpdateEqualizer( 10, 0 );

  return check( expected );
}

BOOL testDistState5( VOID ) {

  WORD expected[ 5 ] = { 6, 12, 19, 25, 32 };

  printf( "Testing 0-5: asc ramp gain: max\n" );
  UpdateEqualizer( 0, 30 );
  UpdateEqualizer( 1, 30 );
  UpdateEqualizer( 2, 40 );
  UpdateEqualizer( 3, 40 );
  UpdateEqualizer( 4, 50 );
  UpdateEqualizer( 5, 50 );
  UpdateEqualizer( 6, 60 );
  UpdateEqualizer( 7, 60 );
  UpdateEqualizer( 8, 70 );
  UpdateEqualizer( 9, 70 );
  UpdateEqualizer( 10, 100 );

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
  failed |= testGainState1();
  failed |= testGainState2();
  failed |= testGainState3();
  failed |= testGainState4();
  failed |= testGainState5();
  failed |= testGainState6();
  failed |= testDistState1();
  failed |= testDistState2();
  failed |= testDistState3();
  failed |= testDistState4();
  failed |= testDistState5();

  free( AmiGUS_MHI_Base );

  return ( failed ) ? 15 : 0;
}
  