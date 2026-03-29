/*
 * This file is part of the amigus.library.
 *
 * amigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * amigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with amigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NULL
#undef NULL
#endif /* NULL */

#include <proto/dos.h>
#include <proto/exec.h>

#include "amigus_private.h"

BOOL CheckStartAddress( LONG address ) {

  BOOL result;

  result = !strncmp(( UBYTE * ) address, 
                     AMIGUS_MEM_LOG_BORDERS, 
                     sizeof( AMIGUS_MEM_LOG_BORDERS ) - 1 );
  if ( !result ) return result;

  address += sizeof( AMIGUS_MEM_LOG_BORDERS ) - 1;
  result = !strncmp(( UBYTE * ) address, 
                     " " STR( LIB_FILE ) " ", 
                     sizeof( " " STR( LIB_FILE ) " " ) - 1 );
  if ( !result ) return result;

  address += sizeof( " " STR( LIB_FILE ) " " ) - 1;
  result = !strncmp(( UBYTE * ) address, 
                     AMIGUS_MEM_LOG_BORDERS, 
                     sizeof( AMIGUS_MEM_LOG_BORDERS ) - 1 );
  return result;
}

VOID WriteMemoryLog( LONG startAddress, STRPTR filename ) {

  BPTR file;
  LONG endAddress = startAddress 
    + sizeof( AMIGUS_MEM_LOG_BORDERS)
    + sizeof( " " STR( LIB_FILE ) " " )
    + sizeof( AMIGUS_MEM_LOG_BORDERS)
    - 2;

  printf( "Found start memMarker at 0x%08lx\n", startAddress );
  file = Open( filename, MODE_NEWFILE );
  if ( !file ) {

    printf( "Opening \"%s\" failed, bailing out...\n", filename );
    return;
  }
  printf( "Incrementally finding end memMarker...\n" );
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
  printf( "Done, find your log at %s\n", filename );
}

int main( int argc, char const *argv[] ) {

  STRPTR filename = "ram:AmiGUS-MemLog.txt";
  LONG startAddress = 0;
  BOOL found = FALSE;
  ULONG i;
  
  /*
  Seems to be in sc.lib already
  DOSBase = ( struct DosLibrary * ) OpenLibrary( "dos.library", 34 );
  if ( !DOSBase ) {
    printf( "Opening dos.library failed.\n" );
    return 20;
  }
  */
  for ( i = 1; i < argc; ++i ) {
    if ( !strncmp( "address=0x", argv[ i ], sizeof( "address=0x" ) - 1 )) {

      STRPTR start = ( STRPTR )(( LONG ) argv[ i ] + sizeof( "address=0x" ) - 1 );
      STRPTR end = ( STRPTR )(( LONG ) start + 8);
      startAddress = strtol( start, ( BYTE ** ) &end, 16 );
      printf( "Trying 0x%08lx...\n", startAddress );
      found = CheckStartAddress( startAddress );
      continue;
    }
    if ( !strncmp( "out=", argv[ i ], sizeof( "out=" ) - 1 )) {

      filename = ( STRPTR )((( ULONG ) argv[ i ] ) + sizeof( "out=" ) - 1 );
      printf( "Using \"%s\" for output...\n", filename );
      continue;
    }
    printf( "Unknown parameter %s\n", argv[ i ] );
    printf( "USAGE:\nGetMemLog [address=0x23443500] [out=t:amigus-memlog.txt]\n" );
    return 0;
  }

#ifdef INCLUDE_VERSION
  if ( !found ) {
    if ( 36 <= (( struct Library * ) DOSBase )->lib_Version) {

      UBYTE buffer[ 64 ];

      printf( "Checking for settings first...\n" );
      i = GetVar( "AmiGUS-LOG-ADDRESS", buffer, sizeof( buffer ), 0 );  
      if ( i > 0 ) {
  
        StrToLong( buffer, ( LONG * ) &startAddress );
        printf( " ... found address: 0x%08lx\n", startAddress );
        found = CheckStartAddress( startAddress );

      } else {

        printf( " ... NO address found\n" );
      }
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

    LONG memStart[ 32 ];
    LONG memEnd[ 32 ];
    struct MemHeader * mem;
    ULONG slab = 0;

    printf( "Trying full scan finally...\n" );
    
    Forbid();
    for ( mem = ( struct MemHeader * ) SysBase->MemList.lh_Head;
          mem->mh_Node.ln_Succ;
          mem = ( struct MemHeader * ) mem->mh_Node.ln_Succ ) {

      memStart[ slab ] = ( LONG )mem;
      memEnd[ slab ] = ( LONG )mem->mh_Upper;
      ++slab;
    }
    Permit();
    for ( i = 0; (( i < slab ) && ( !found )); i++ ) {

      printf( "Scanning region 0x%08lx to 0x%08lx...\n",
              memStart[ i ], memEnd[ i ] );
      for ( startAddress = memStart[ i ]; 
            (( startAddress < memEnd[ i ] ) && ( !found ));
            startAddress += 4 ) {

        if ( CheckStartAddress( startAddress )) {

          printf( "Success at 0x%08lx\n", startAddress );
          found = TRUE;
        } else if ( 0 == ( startAddress & 0x000fFFff )) {

          printf( "\r...now at 0x%08lx...\n", startAddress);
        } 
      }
    }
  }

  if (( found ) && ( startAddress )) {

    WriteMemoryLog( startAddress, filename );

  } else {

    printf( "No start marker found, giving up, sorry\n" );
  }
  /*
  Seems to be in sc.lib already
  if ( DOSBase ) {
    CloseLibrary(( struct Library * ) DOSBase );
    DOSBase = NULL;
  }
  */
  return 0;
}
