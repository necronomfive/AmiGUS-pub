/*
 * This file is part of the FindAmiGUS Utility.
 *
 * FindAmiGUS Utility is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * FindAmiGUS Utility is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FindAmiGUS Utility.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <amigus/amigus.h>

#include <proto/amigus.h>
#include <proto/dos.h>
#include <proto/exec.h>

// Result bitmasks!
#define AMIGUS_ZORRO2     0x000000001
#define AMIGUS_MINI       0x000000002

/******************************************************************************
 * Private functions.
 *****************************************************************************/

/**
 * Like string.h's stricmp.
 * Returns the case-insensitive comparison result of the two strings.
 *
 * @param a
 * @param b
 *
 * @return -1 if a < b,
 *          0 if a = b,
 *          1 if a > b.
 */
static LONG C_stricmp(STRPTR a, STRPTR b) {

  LONG result;
  while (
    !(result = (0xDF & ( *a )) - (0xDF & ( *b ))) &&
    (*a++) &&
    (*b++)
  );
  return (result > 0) - (result < 0);
}

/******************************************************************************
 * Entry point.
 *****************************************************************************/

int main( int argc, char **argv ) {

  const STRPTR AMIGUS_LIBRARY = "amigus.library";

  struct Library * AmiGUS_Base = NULL;
  struct AmiGUS * amigus = NULL;
  BOOL talkative = !(( argc >= 2 ) && ( !( C_stricmp( argv[ 1 ], "QUIET" ))));
  int result = 0;

  AmiGUS_Base = OpenLibrary( AMIGUS_LIBRARY, 0 );
  if ( !( AmiGUS_Base )) {

    if ( talkative ) {

      Printf( "Error: Cannot open %s.\n", AMIGUS_LIBRARY );
    }
    return 60;
  }

  while ( amigus = AmiGUS_FindCard( amigus )) {

    switch ( amigus->agus_TypeId ) {
      case AmiGUS_Zorro2: {
        
        result |= AMIGUS_ZORRO2;
        break;
      }
      case AmiGUS_mini: {
        
        result |= AMIGUS_MINI;
        break;
      }
      default: {

        Printf( "Found unknown AmiGUS TypeId 0x%08lx"
                " - does FindAmiGUS need an update?\n",
                amigus->agus_TypeId );
        continue;
      }
    }

    Printf( "Found %s PCM @ 0x%08lx, Wavetable @ 0x%08lx, Codec @ 0x%08lx.\n",
      amigus->agus_TypeName,
      amigus->agus_PcmBase,
      amigus->agus_WavetableBase,
      amigus->agus_CodecBase );
  }

  CloseLibrary( AmiGUS_Base );

  if ( talkative ) {
    if ( !( result )) {

      Printf( "No AmiGUS found.\n" );
    }
    Printf( "Returning 0x%08lx\n", result );
  }

  return result;
}
