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

 /******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
#include <stdio.h>
#include <string.h>

#ifdef NULL
#undef NULL
#endif /* NULL */

#include <proto/dos.h>
#include <proto/exec.h>

#include "amigus_mhi.h"


// vamos sc IDIR=header OBJNAME test/get_mem_log.o test/get_mem_log.c 
// vamos slink NOICONS TO test/GetMemLog FROM LIB:c.o test/get_mem_log.o LIB LIB:sc.lib
// cp test/GetMemLog ~/Documents/FS-UAE/Shared/MHI/

UBYTE marker[] = AMIGUS_MEM_LOG_MARKER;

BOOL CheckStartAddress( LONG address ) {

  BOOL result = FALSE;

  if (( UBYTE * ) address != marker ) {

    result = !strncmp(( UBYTE * ) address, marker, sizeof( marker) - 1 );
  }
  return result;
}

VOID WriteMemoryLog( LONG startAddress, STRPTR filename ) {

  BPTR file = NULL;
  LONG endAddress = startAddress + sizeof( marker);

  printf( "Found start marker at 0x%08lx\n", startAddress );
  file = Open( filename, MODE_NEWFILE );
  if ( !file ) {

    printf( "Opening \"%s\" failed, bailing out...\n", filename );
    return;
  }
  printf( "Incrementally finding end marker...\n" );
  while ( *(( UBYTE * ) endAddress )) {
    if ( 0x1000 <= ( endAddress - startAddress )) {

      printf( "... writing %ld bytes increment...\n",
              endAddress - startAddress );
      Write ( file, ( APTR ) startAddress, endAddress - startAddress );
      startAddress = endAddress;
    }
    ++endAddress;
  }
  printf( "... writing remaining %ld bytes.\n",
          endAddress - startAddress );
  Write( file, ( APTR ) startAddress, endAddress - startAddress );
  Close( file );
  file = NULL;
  printf( "Done, find your log at %s\n", filename );
}

int main( int argc, char const *argv[] ) {

  UBYTE * filename = "ram:MemLog.txt";
  LONG startAddress;
  
  BOOL found = FALSE;

  DOSBase = ( struct DosLibrary * ) OpenLibrary( "dos.library", 34 );
  if ( !DOSBase ) {
    printf( "Opening dos.library failed.\n" );
    return 20;
  }

  printf( "Checking for settings first...\n" );
#ifdef INCLUDE_VERSION
  if ( 36 <= (( struct Library * ) DOSBase )->lib_Version) {
    UBYTE buffer[ 64 ];
    LONG i = GetVar( "AmiGUS-MHI-LOG-ADDRESS", buffer, sizeof( buffer ), 0 );

    if ( i > 0 ) {

      StrToLong( buffer, ( LONG * ) &startAddress );
      printf( " ... found address: 0x%08lx\n", startAddress );
      found = CheckStartAddress( startAddress );
    } else {
      printf( " ... NO address found\n" );
    }
  }
#endif
  
  if ( !( found )) {

    printf( "Trying default locations next...\n" );
    startAddress = 0x0a000000;
    printf( " ... 3000/4000 CPU slot: 0x%08lx\n", startAddress );
    found = CheckStartAddress( startAddress );
  }
  if ( !( found )) {

    startAddress = 0x00400000;
    printf( " ... Zorro II : 0x%08lx\n", startAddress );
    found = CheckStartAddress( startAddress );
  }
  if ( !( found )) {

    startAddress = 0x48000000;
    printf( " ... Zorro III : 0x%08lx\n", startAddress );
    found = CheckStartAddress( startAddress );
  }
  if ( !( found )) {

    printf( "Trying full scan finally...\n" );
    // TODO: list memory regions, scan them LONG wise
  }

  if ( found ) {

    WriteMemoryLog( startAddress, filename );

  } else {

    printf( "No start marker found, giving up, sorry\n" );
  }
  return 0;
}
