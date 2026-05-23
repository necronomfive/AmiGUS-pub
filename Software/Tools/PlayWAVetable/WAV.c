/*
 * This file is part of the AmiGUS PlayWAVetable Utility.
 *
 * AmiGUS PlayWAVetable Utility is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS PlayWAVetable Utility is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AmiGUS PlayWAVetable Utility.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <exec/memory.h>

#include <proto/dos.h>
#include <proto/exec.h>

#include "WAV.h"

/******************************************************************************
 * WAV format definitions below.
 *****************************************************************************/

#define CHAR_TO_ULONG( a, b, c, d ) ((( a ) << 24 ) | (( b ) << 16 ) | \
                                     (( c ) << 8 ) | ( d ))

#define RIFF_ID                     CHAR_TO_ULONG( 'R', 'I', 'F', 'F' )
#define WAVE_ID                     CHAR_TO_ULONG( 'W', 'A', 'V', 'E' )
#define FMT_ID                      CHAR_TO_ULONG( 'f', 'm', 't', ' ' )
#define DATA_ID                     CHAR_TO_ULONG( 'd', 'a', 't', 'a' )

#define HEADER_SIZE                 44

/******************************************************************************
 * Implementation specific definitions below.
 *****************************************************************************/

#define BUFFER_SIZE                 4096

/******************************************************************************
 * WAV is a x86 format, welcome to endianess-hell and the helper functions.
 ******************************************************extrem***********************/

UWORD Swap16( UWORD value ) {

  return ( UWORD )((( value & 0xFF00 ) >> 8 ) |
                   (( value & 0x00FF ) << 8 ));
}

ULONG Swap32( ULONG value ) {

  return ((( value & 0xFF000000 ) >> 24 ) |
          (( value & 0x00FF0000 ) >>  8 ) |
          (( value & 0x0000FF00 ) <<  8 ) |
          (( value & 0x000000FF ) << 24 ));
}

/*****************************************************
 * WAV file loading and parsing functions below.
 *****************************************************/

LONG OpenWav( struct WAV * wav, STRPTR filename ) {

  LONG result;
  LONG position = 0;

  BYTE header[ HEADER_SIZE ];

  wav->WAV_file = NULL;
  wav->WAV_buffer = NULL;

  wav->WAV_file = Open( filename, MODE_OLDFILE );
  if ( !( wav->WAV_file )) {

    CloseWav( wav );
    return 1; // Could not open file.
  }

  result = Read( wav->WAV_file, header, HEADER_SIZE );
  if ( HEADER_SIZE != result ) {

    CloseWav( wav );
    return 2; // Could not read header.
  }

  wav->WAV_buffer = AllocMem( BUFFER_SIZE, MEMF_ANY );
  if ( !wav->WAV_buffer ) {

    CloseWav( wav );
    return 3; // Could not alloc memory.
  }

  if ( RIFF_ID != ( * (( LONG * )( &( header[ position ] ))))) {

    CloseWav( wav );
    return 3; // Invalid file format 1
  }
  // FileSize - ignored
  position += 8;

  if ( WAVE_ID != ( * (( LONG * )( &( header[ position ] ))))) {

    CloseWav( wav );
    return 4; // Invalid file format 2
  }
  position += 4;

  if ( FMT_ID != ( * (( LONG * )( &( header[ position ] ))))) {

    CloseWav( wav );
    return 5; // Invalid file format 3
  }
  // BlocSize - ignored
  position += 8;

  if ( 1 != Swap16( * (( WORD * )( &( header[ position ] ))))) {

    CloseWav( wav );
    return 6; // Not a PCM WAV file
  }
  position += 2;

  wav->WAV_Channels = Swap16( * (( WORD * )( &( header[ position ] ))));
  if (( 1 > wav->WAV_Channels ) || ( 2 < wav->WAV_Channels )) {

    CloseWav( wav );
    return 7; // Invalid channels
  }
  position += 2;

  wav->WAV_SampleRate = Swap32( * (( ULONG * )( &( header[ position ] ))));
  position += 10;
  // BytePerSec and BytePerBloc - ignored

  wav->WAV_SampleBits = Swap16( * (( WORD * )( &( header[ position ] ))));
  position += 2;

  if ( DATA_ID != ( * (( LONG * )( &( header[ position ] ))))) {

    CloseWav( wav );
    return 8; // Cannot read data size
  }
  position += 4;

  wav->WAV_DataSize = Swap32( * (( ULONG * )( &( header[ position ] ))));
  position += 4;

  wav->WAV_DataOffset = position;

  return 0; // success!
}

VOID CloseWav( struct WAV * wav ) {

  if ( wav->WAV_buffer ) {

    FreeMem( wav->WAV_buffer, BUFFER_SIZE );
    wav->WAV_buffer = NULL;
  }
  if ( wav->WAV_file ) {

    Close( wav->WAV_file );
    wav->WAV_file = NULL;
  }
}

LONG ReadChunkLE( struct WAV * wav ) {

  LONG result = Read( wav->WAV_file, wav->WAV_buffer, BUFFER_SIZE );
  return result;
}

LONG ReadChunkBE( struct WAV * wav ) {

  LONG i;
  LONG result = ReadChunkLE( wav );
  if (( !( result )) || ( 8 == wav->WAV_SampleBits )) {

    return result;
  }

  for ( i = 0; i < ( BUFFER_SIZE >> 1 ); ++i ) {

    WORD * buffer = ( WORD * ) wav->WAV_buffer;
    buffer[ i ] = Swap16( buffer[i] );
  }

  return result;
}
