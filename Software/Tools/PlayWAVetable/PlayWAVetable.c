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

#include <amigus/amigus.h>
#include <exec/memory.h>

#include <proto/amigus.h>
#include <proto/dos.h>
#include <proto/exec.h>

/******************************************************************************
 * WAV format definitions below.
 *****************************************************************************/

#define CHAR_TO_ULONG( a, b, c, d ) ((( a ) << 24 ) | (( b ) << 16 ) | (( c ) << 8 ) | ( d ))

#define RIFF_ID   CHAR_TO_ULONG( 'R', 'I', 'F', 'F' )
#define WAVE_ID   CHAR_TO_ULONG( 'W', 'A', 'V', 'E' )
#define FMT_ID    CHAR_TO_ULONG( 'f', 'm', 't', ' ' )
#define DATA_ID   CHAR_TO_ULONG( 'd', 'a', 't', 'a' )

/******************************************************************************
 * WAV is a x86 format, welcome to endianess-hell and the helper functions.
 *****************************************************************************/

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

/******************************************************************************
 * AmiGUS Wavetable hardware definitions below
 *****************************************************************************/

#define AMIGUS_WT_RESET                 0x18

#define AMIGUS_WT_ADDRESS_32BIT         0x14
#define AMIGUS_WT_DATA_32BIT            0x10

#define AMIGUS_WT_ADDRESS_HIGH          0x14
#define AMIGUS_WT_ADDRESS_LOW           0x16
#define AMIGUS_WT_DATA_HIGH             0x10
#define AMIGUS_WT_DATA_LOW              0x12

#define AMIGUS_WT_CHANNEL_NUMBER        0x1e
#define AMIGUS_WT_CHANNEL_CONTROL       0x20

#define AMIGUS_WT_CHANNEL_START_32BIT   0x22
#define AMIGUS_WT_CHANNEL_LOOP_32BIT    0x26
#define AMIGUS_WT_CHANNEL_END_32BIT     0x2a
#define AMIGUS_WT_CHANNEL_RATE_32BIT    0x2e

#define AMIGUS_WT_CHANNEL_START_HIGH    0x22
#define AMIGUS_WT_CHANNEL_START_LOW     0x24
#define AMIGUS_WT_CHANNEL_LOOP_HIGH     0x26
#define AMIGUS_WT_CHANNEL_LOOP_LOW      0x28
#define AMIGUS_WT_CHANNEL_END_HIGH      0x2a
#define AMIGUS_WT_CHANNEL_END_LOW       0x2c
#define AMIGUS_WT_CHANNEL_RATE_HIGH     0x2e
#define AMIGUS_WT_CHANNEL_RATE_LOW      0x30

#define AMIGUS_WT_CHANNEL_ATTACK        0x36
#define AMIGUS_WT_CHANNEL_DECAY         0x38
#define AMIGUS_WT_CHANNEL_SUSTAIN       0x3a
#define AMIGUS_WT_CHANNEL_RELEASE       0x3c

#define AMIGUS_WT_CHANNEL_VOLUME_LEFT   0x32
#define AMIGUS_WT_CHANNEL_VOLUME_RIGHT  0x34

#define AMIGUS_WT_MASTER_VOLUME_LEFT    0x40
#define AMIGUS_WT_MASTER_VOLUME_RIGHT   0x42

/* FIFO Reset */
#define AMIGUS_WT_F_RESET_STROBE        0x0000

#define AMIGUS_WT_F_RESOLUTION_16BIT    0x0001
#define AMIGUS_WT_F_LOOPED              0x0002
#define AMIGUS_WT_F_INTERPOLATE         0x0004
#define AMIGUS_WT_F_LITTLE_ENDIAN       0x0008
#define AMIGUS_WT_F_ENVELOPE_MODULATION 0x0010
#define AMIGUS_WT_F_ENVELOPE_KEY        0x4000
#define AMIGUS_WT_F_START               0x8000

/******************************************************************************
 * Low-Level hardware access functions.
 *****************************************************************************/

VOID WriteReg16( APTR card, ULONG offset, UWORD value ) {

  *(( UWORD * )(( ULONG ) card + offset )) = value;
}

VOID WriteReg32( APTR card, ULONG offset, ULONG value ) {

  *(( ULONG * )(( ULONG ) card + offset )) = value;
}

/******************************************************************************
 * Mid-Level hardware access functions.
 *****************************************************************************/

VOID InitAmiGus( APTR card ) {

  WORD i;

  WriteReg16( card, AMIGUS_WT_RESET, AMIGUS_WT_F_RESET_STROBE );
  for ( i = 0; i < 32; ++i ) {

    WriteReg16( card, AMIGUS_WT_CHANNEL_NUMBER, i );
    WriteReg16( card, AMIGUS_WT_CHANNEL_CONTROL, 0x0000 );
    WriteReg16( card, AMIGUS_WT_CHANNEL_START_HIGH, 0x0000 );
    WriteReg16( card, AMIGUS_WT_CHANNEL_START_LOW,  0x0000 );
    WriteReg16( card, AMIGUS_WT_CHANNEL_LOOP_HIGH,  0x0000 );
    WriteReg16( card, AMIGUS_WT_CHANNEL_LOOP_LOW,   0x0000 );
    WriteReg32( card, AMIGUS_WT_CHANNEL_END_32BIT, 0x00000000 );
    WriteReg32( card, AMIGUS_WT_CHANNEL_RATE_32BIT, 0x00000000 );
    WriteReg16( card, AMIGUS_WT_CHANNEL_VOLUME_LEFT, 0x8000 );
    WriteReg16( card, AMIGUS_WT_CHANNEL_VOLUME_RIGHT, 0x8000 );
  }
  // Set master volume
  WriteReg16( card, AMIGUS_WT_MASTER_VOLUME_LEFT, 0xffff );
  WriteReg16( card, AMIGUS_WT_MASTER_VOLUME_RIGHT, 0xffff );
  Printf( "Wavetable @ 0x%08lx init'ed\n", card );
}

VOID LoadAmiGusWavetableSample( APTR card, ULONG target, ULONG data ) {

  WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, target );
  WriteReg32( card, AMIGUS_WT_DATA_32BIT, data );
}

VOID StartAmiGusWavetablePlayback( APTR card,
                                   UWORD channels,
                                   ULONG sampleRate,
                                   UWORD resolution,
                                   ULONG count ) {

  ULONG middle = 1 << 24;
  ULONG end = 1 << 25;
  double maxFreq = 0x40000000;
	double maxRate = 192000;
	ULONG frequency = (( double ) sampleRate ) * maxFreq / maxRate;
  UWORD channel0VolumeRight = ( 2 == channels ) ? 0 : 0xFFFF;

  resolution = ( 16 == resolution ) 
               ? ( AMIGUS_WT_F_LITTLE_ENDIAN | AMIGUS_WT_F_RESOLUTION_16BIT )
               : 0;

  Printf( "Calculated frequency is 0x%08lx for %ldHz.\n",
          frequency,
          sampleRate );

  // Mono plays on channel 0 on both speakers, 
  // but only left stereo channel goes to channel 0 left.

  WriteReg16( card, AMIGUS_WT_CHANNEL_NUMBER, 0x0000 );
  WriteReg16( card, AMIGUS_WT_CHANNEL_CONTROL, 0x0000 );
  WriteReg32( card, AMIGUS_WT_CHANNEL_RATE_32BIT, frequency );
  WriteReg16( card, AMIGUS_WT_CHANNEL_VOLUME_LEFT, 0xFFFF );
  WriteReg16( card, AMIGUS_WT_CHANNEL_VOLUME_RIGHT, channel0VolumeRight );
  
  WriteReg32( card, AMIGUS_WT_CHANNEL_START_32BIT, 0 );
  WriteReg32( card, AMIGUS_WT_CHANNEL_END_32BIT, count );
 
  WriteReg16( card,
              AMIGUS_WT_CHANNEL_CONTROL,
              AMIGUS_WT_F_START
              | AMIGUS_WT_F_INTERPOLATE
              | resolution );

  if ( channels == 1 ) {

    return;
  }

  // Only right stereo channel goes to channel 1 right.

  WriteReg16( card, AMIGUS_WT_CHANNEL_NUMBER, 0x0001 );
  WriteReg16( card, AMIGUS_WT_CHANNEL_CONTROL, 0x0000 );
  WriteReg32( card, AMIGUS_WT_CHANNEL_RATE_32BIT, frequency );
  WriteReg16( card, AMIGUS_WT_CHANNEL_VOLUME_LEFT, 0 );
  WriteReg16( card, AMIGUS_WT_CHANNEL_VOLUME_RIGHT, 0xFFFF );
  
  WriteReg32( card, AMIGUS_WT_CHANNEL_START_32BIT, middle );
  WriteReg32( card, AMIGUS_WT_CHANNEL_END_32BIT, middle + count );
 
  WriteReg16( card,
              AMIGUS_WT_CHANNEL_CONTROL,
              AMIGUS_WT_F_START
              | AMIGUS_WT_F_INTERPOLATE
              | resolution );
}

/******************************************************************************
 * Entry point.
 *****************************************************************************/

int main( int argc, char **argv ) {

  struct Library * AmiGUS_Base = NULL;
  struct AmiGUS * amigus = NULL;
  BPTR file = NULL;
  UBYTE * buffer = NULL;
  struct Task * mainTask = FindTask( NULL );
  ULONG samplerate;
  UWORD channels;
  UWORD resolution;
  ULONG datasize;
  ULONG posA = 0;
  ULONG posB = 1 << 24;
  ULONG tempL;
  LONG available;
  UWORD tempW;
  int result = 0;
  LONG copyIncrement;
  APTR card;

  Printf( "\n============================="
          "\n  AmiGUS PlayWAVetable V0.1  "
          "\n============================="
          "\n"
          "\nUsage: PlayWAVetable some.wav"
          "\n"
          "\nKeep in mind: only WAV files < 32MB supported!"
          "\n" );

  if ( argc < 2 ) {

    Printf( "No WAV file name provided.\n" );
    result = 30;
    goto cleanup;
  }
  AmiGUS_Base = OpenLibrary( "amigus.library", 1 );
  if ( !AmiGUS_Base ) {

    Printf( "Could not open amigus.library.\n" );
    result = 31;
    goto cleanup;
  }

  /*****************************************************************************
   * Detecting and owning AmiGUS WaveTable part.
   ****************************************************************************/

  amigus = AmiGUS_FindCard( NULL );
  if ( !amigus ) {

    Printf( "No AmiGUS found.\n" );
    result = 32;
    goto cleanup;
  }

  tempL = AmiGUS_ReserveCard( amigus, AMIGUS_FLAG_WAVETABLE, mainTask );
  if ( tempL ) {

    Printf( "Could not reserve AmiGUS at 0x%08lx.\n",
            amigus->agus_WavetableBase );
    result = 33;
    goto cleanup;
  }
  card = amigus->agus_WavetableBase;

  /*****************************************************************************
   * Flushing AmiGUS WaveTable hardware.
   ****************************************************************************/

  InitAmiGus( card );

  /*****************************************************************************
   * Reading and parsing WAV files... Kudos to wikipedia ;-)
   ****************************************************************************/

  file = Open( argv[ 1 ], MODE_OLDFILE );
  if ( !file ) {

    Printf( "Could not open file '%s'.\n", argv[ 1 ]);
    result = 34;
    goto cleanup;
  }

  buffer = AllocMem( 4096, MEMF_ANY );

  if ( !buffer ) {

    Printf( "Could not allocate buffer.\n" );
    result = 35;
    goto cleanup;
  }

  Read( file, &tempL, 4 );
  if ( RIFF_ID != tempL ) {

    Printf( "Invalid file format.\n" );
    result = 36;
    goto cleanup;
  }

  Seek( file, 4, OFFSET_CURRENT ); // FileSize - ignored

  Read( file, &tempL, 4 );
  if ( WAVE_ID != tempL ) {

    Printf( "Invalid file format.\n" );
    result = 38;
    goto cleanup;
  }

  Read( file, &tempL, 4 );
  if ( FMT_ID != tempL ) {

    Printf( "Invalid file format.\n" );
    result = 39;
    goto cleanup;
  }

  Seek( file, 4, OFFSET_CURRENT ); // BlocSize - ignored

  Read( file, &tempW, 2 );
  if ( 1 != Swap16( tempW )) {

    Printf( "Invalid file format 0x%08lx - need PCM.\n", tempW );
    result = 40;
    goto cleanup;
  }

  Read( file, &tempW, 2 );
  channels = Swap16( tempW );
  if (( 1 > channels ) || ( 2 < channels )) {

    Printf( "Can only do 2 channels yet, got %ld.\n", channels );
    result = 41;
    goto cleanup;
  }

  Read( file, &tempL, 4 );
  samplerate = Swap32( tempL );

  Seek( file, 6, OFFSET_CURRENT ); // BytePerSec and BytePerBloc - ignored

  Read( file, &tempW, 2 );
  resolution = Swap16( tempW );

  Read( file, &tempL, 4 );
  if ( DATA_ID != tempL ) {

    Printf( "Invalid file format, expected 'data'.\n" );
    result = 42;
    goto cleanup;
  }

  Read( file, &tempL, 4 );
  datasize = Swap32( tempL );
  if (( 1 << 25 ) < datasize ) {

    Printf( "File too big.\n" );
    result = 37;
    goto cleanup;
  }

  Printf( "Found %ldbit, %ldHz, %ld channels, %ld total bytes.\n",
          resolution, samplerate, channels, datasize );

  Printf( "Playing on card 0x%08lx (eb0000)\n", card );

  /*****************************************************************************
   * This is the copy and re-sort function for all bytes from the WAV.
   * Why?
   * - AmiGUS WaveTable needs 1 stream of consecutive samples PER VOICE,
   *   while WAV comes with interleaved samples, hence the inefficient
   *   copy LONG by LONG into AmiGUS memory
   * - AmiGUS WaveTable is always signed,
   *   WAV 8bit is unsigned, 16 bit is signed.
   ****************************************************************************/

  if ( 1 == channels ) {

    // Mono can go w/o re-sorting samples...
    copyIncrement = 4;

  } else {

    // Stereo cannot, need the bytes per sample:
    copyIncrement = resolution >> 3;
  }

  do {
    LONG i;
    ULONG valueA = 0;
    ULONG valueB = 0;
    BYTE countA = 0;
    BYTE countB = 0;

    available = Read( file, buffer, 4096 );
    for ( i = 0; i < available; ++i ) {

      //Printf( "Next byte 0x%02lx\n", buffer[ i ] );
      // Fix up signed vs unsigned for 8bit only
      if ( 8 == resolution ) {

        buffer[ i ] -= 128;
      }
      // Sort 32bit of samples together, no matter how many these are
      if (( 2 == channels ) && ( i & copyIncrement )) {

        valueB = ( valueB  << 8 ) | buffer[ i ];
        ++countB;

      } else {

        valueA = ( valueA  << 8 ) | buffer[ i ];
        ++countA;
      }
      // And copy them to WaveTable memory.
      // AmiGUS needs always 32bit per go...
      if ( 4 == countA ) {

        //Printf( "Writing A=0x%08lx\n", valueA );
        LoadAmiGusWavetableSample( card, posA, valueA );
        countA = 0;
        valueA = 0;
        posA += 4;
      }
      if ( 4 == countB ) {

        //Printf( "Writing B=0x%08lx\n", valueB );
        LoadAmiGusWavetableSample( card, posB, valueB );
        countB = 0;
        valueB = 0;
        posB += 4;
      }
    }
  } while( 0 < available );

  /*****************************************************************************
   * Starting playback finally... Easy, eh?
   ****************************************************************************/

  StartAmiGusWavetablePlayback( card,
                                channels,
                                samplerate,
                                resolution,
                                posA - 4 );

  Printf( "Did you know? AmiGUS can play alone...\n" );

  /*****************************************************************************
   * Sorry for the ... hack.
   ****************************************************************************/
// Yes, this is awful, but keeps this demo code readable...
cleanup:
  if ( buffer ) {

    FreeMem( buffer, 4096 );
  }
  if ( file ) {

    Close( file );
  }
  if ( amigus ) {

    AmiGUS_FreeCard( amigus, AMIGUS_FLAG_WAVETABLE, mainTask );
  }
  if ( AmiGUS_Base ) {

    CloseLibrary( AmiGUS_Base );
  }

  return result;
}
