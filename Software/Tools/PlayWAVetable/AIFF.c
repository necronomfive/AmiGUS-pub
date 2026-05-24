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

#include "AIFF.h"

/******************************************************************************
 * AIFF format definitions below.
 *****************************************************************************/

#define CHAR_TO_ULONG( a, b, c, d ) ((( a ) << 24 ) | (( b ) << 16 ) | \
                                     (( c ) << 8 ) | ( d ))

#define FORM_ID                     CHAR_TO_ULONG( 'F', 'O', 'R', 'M' )
#define AIFF_ID                     CHAR_TO_ULONG( 'A', 'I', 'F', 'F' )
#define COMM_ID                     CHAR_TO_ULONG( 'C', 'O', 'M', 'M' )
#define SSND_ID                     CHAR_TO_ULONG( 'S', 'S', 'N', 'D' )

#define HEADER_SIZE                 54

/******************************************************************************
 * Implementation specific definitions below.
 *****************************************************************************/

// BUFFER_SIZE is approximately 8k and nicely devisable by 
// 1 for 8bit Mono
// 2 for 8bit Stereo and 16bit Mono
// 3 for 24bit Mono
// 4 for 16bit Stereo and
// 6 for 24bit Stereo
#define BUFFER_SIZE                 8184

/******************************************************************************
 * AIFF is a m68 format, nice big endian and all cool.
 * But maybe we need little endian every now and then?
 *****************************************************************************/

static UWORD Swap16( UWORD value ) {

  return ( UWORD )((( value & 0xFF00 ) >> 8 ) |
                   (( value & 0x00FF ) << 8 ));
}

static ULONG Swap32( ULONG value ) {

  return ((( value & 0xFF000000 ) >> 24 ) |
          (( value & 0x00FF0000 ) >>  8 ) |
          (( value & 0x0000FF00 ) <<  8 ) |
          (( value & 0x000000FF ) << 24 ));
}

/*****************************************************
 * AIFF file loading and parsing functions below.
 *****************************************************/

ULONG GetSampleRate( UBYTE * bytes ) {

  LONG exponent = ((( bytes[ 0 ] & 0x7f ) << 8 ) | bytes[ 1 ] ) - 16383;
  ULONG mantissa_high = ( bytes[ 2 ] << 24 ) |
                        ( bytes[ 3 ] << 16 ) |
                        ( bytes[ 4 ] <<  8 ) |
                        ( bytes[ 5 ]       );
  ULONG result = mantissa_high >> ( 31 - exponent );

  if (( 0 > exponent ) | ( 31 < exponent )) {

    return -1;
  }

  return result;
}

LONG OpenAiff( struct aiff * aiff, STRPTR filename ) {

  LONG result;
  LONG position = 0;

  BYTE header[ HEADER_SIZE ];

  aiff->aiff_File = NULL;
  aiff->aiff_Buffer = NULL;

  aiff->aiff_File = Open( filename, MODE_OLDFILE );
  if ( !( aiff->aiff_File )) {

    CloseAiff( aiff );
    return 1; // Could not open file.
  }

  result = Read( aiff->aiff_File, header, HEADER_SIZE );
  if ( HEADER_SIZE != result ) {

    CloseAiff( aiff );
    return 2; // Could not read header.
  }

  aiff->aiff_Buffer = AllocMem( BUFFER_SIZE, MEMF_ANY );
  if ( !aiff->aiff_Buffer ) {

    CloseAiff( aiff );
    return 3; // Could not alloc memory.
  }

  if ( FORM_ID != ( * (( LONG * )( &( header[ position ] ))))) {

    CloseAiff( aiff );
    return 4; // Invalid file format 1
  }
  // FileSize - ignored
  position += 8;

  if ( AIFF_ID != ( * (( LONG * )( &( header[ position ] ))))) {

    CloseAiff( aiff );
    return 5; // Invalid file format 2
  }
  position += 4;

  if ( COMM_ID != ( * (( LONG * )( &( header[ position ] ))))) {

    CloseAiff( aiff );
    return 6; // Invalid file format 3
  }
  // ChunkSize - ignored
  position += 8;

  aiff->aiff_Channels = ( * (( WORD * )( &( header[ position ] ))));
  if (( 1 > aiff->aiff_Channels ) || ( 2 < aiff->aiff_Channels )) {

    CloseAiff( aiff );
    return 8; // Invalid channels
  }
  // SamplesPerChannel - ignored
  position += 6;

  aiff->aiff_SampleBits = ( * (( WORD * )( &( header[ position ] ))));
  position += 2;

  aiff->aiff_SampleRate = GetSampleRate( &( header[ position ] ));
  if (( 0 >= aiff->aiff_SampleRate ) || ( 192000 < aiff->aiff_SampleRate )) {

    CloseAiff( aiff );
    return 9;
  }
  position += 10;

  if ( SSND_ID != ( * (( LONG * )( &( header[ position ] ))))) {

    CloseAiff( aiff );
    return 10; // Invalid file format 4
  }
  position += 4;

  // TODO: -8 line below?
  aiff->aiff_DataSize = ( * (( ULONG * )( &( header[ position ] ))));
  position += 4;

  if ( 0 != ( * (( LONG * )( &( header[ position ] ))))) {

    CloseAiff( aiff );
    return 11; // Not a 0 offset file
  }
  position += 4;

  if ( 0 != ( * (( LONG * )( &( header[ position ] ))))) {

    CloseAiff( aiff );
    return 12; // Not a 0 BlockSize file
  }
  position += 4;

  aiff->aiff_DataOffset = position;

  return 0; // success!
}

VOID CloseAiff( struct aiff * aiff ) {

  if ( aiff->aiff_Buffer ) {

    FreeMem( aiff->aiff_Buffer, BUFFER_SIZE );
    aiff->aiff_Buffer = NULL;
  }
  if ( aiff->aiff_File ) {

    Close( aiff->aiff_File );
    aiff->aiff_File = NULL;
  }
}

LONG ReadAiffChunkLE( struct aiff * aiff ) {

  LONG i;
  LONG result = ReadAiffChunkBE( aiff );
  if (( !( result )) || ( 8 == aiff->aiff_SampleBits )) {

    return result;
  }

  for ( i = 0; i < ( BUFFER_SIZE >> 1 ); ++i ) {

    WORD * buffer = ( WORD * ) aiff->aiff_Buffer;
    buffer[ i ] = Swap16( buffer[i] );
  }

  return result;
}

LONG ReadAiffChunkBE( struct aiff * aiff ) {

  LONG result = Read( aiff->aiff_File, aiff->aiff_Buffer, BUFFER_SIZE );
  return result;
}
