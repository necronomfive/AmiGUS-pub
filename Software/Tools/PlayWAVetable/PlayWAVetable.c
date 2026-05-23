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

#include "WAV.h"

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

VOID StartAmiGusWavetablePlayback( APTR card,
                                   UWORD channels,
                                   ULONG sampleRate,
                                   UWORD resolution,
                                   ULONG count ) {

  ULONG middle = 1 << 24;
  // ULONG end = 1 << 25;
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
  struct Task * mainTask = FindTask( NULL );
  struct wav wav;
  ULONG positionA = 0;
  ULONG positionB = 1 << 24;
  LONG available;
  LONG result;
  APTR card;

  Printf( "\n============================="
          "\n  AmiGUS PlayWAVetable V0.2  "
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

  result = AmiGUS_ReserveCard( amigus, AMIGUS_FLAG_WAVETABLE, mainTask );
  if ( result ) {

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

  result = OpenWav( &wav, argv[ 1 ]);
  if ( result ) {

    Printf( "Cannot read file.\n" );
    result = 34;
    goto cleanup;
  }
  if (( 1 << 25 ) < wav.wav_DataSize ) {

    Printf( "File too big, max %ld but is %ld bytes.\n",
            ( 1 << 25 ),
            wav.wav_DataSize );
    result = 35;
    goto cleanup;
  }

  Printf( "Found %ldbit, %ldHz, %ld channels, %ld total bytes.\n",
          wav.wav_SampleBits,
          wav.wav_SampleRate,
          wav.wav_Channels,
          wav.wav_DataSize );

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

  while ( available = ReadChunkLE( &wav )) {

    LONG i;

    if ( 8 == wav.wav_SampleBits ) {

      // unsigned -> signed conversion for the next block of data!

      UBYTE * buffer = wav.wav_Buffer;

      for ( i = 0; i < available; ++i ) {
        // Fix up signed vs unsigned for 8bit only

        buffer[ i ] -= 128;
      }
    }

    if ( 1 == wav.wav_Channels ) {

      // 8bit or 16bit MONO - No need to fiddle around, can just be coppied

      ULONG * buffer = wav.wav_Buffer;

      WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, positionA );
      for ( i = 0; i < ( available >> 2 ); ++i ) {

        WriteReg32( card, AMIGUS_WT_DATA_32BIT, buffer[ i ]);
      }
      positionA += available;
      // Could also rely on auto-increment as it is only one channel,
      // but we love symmetric code, don't we?

    } else if ( 8 == wav.wav_SampleBits ) {

      // 8bit STEREO - resort bytes from block into channels

      UBYTE * buffer = wav.wav_Buffer; // 1 Sample = 1 UBYTE
      ULONG data;

      // Channel A
      WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, positionA );
      data = 0;
      for ( i = 0; i < available; i += 2 ) {

        data = ( data << 8 );
        data |= buffer[ i ];
        // since buffer is UBYTE *, i is addressing BYTEs, too
        if (( i & 6 ) == 6 ) {

          // Triggered every 4 bytes with the i += 2 above!
          WriteReg32( card, AMIGUS_WT_DATA_32BIT, data );
          data = 0;
        }
      }
      positionA += ( available >> 1 );

      // Channel B
      WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, positionB );
      data = 0;
      for ( i = 1; i < available; i += 2 ) {

        data = ( data << 8 );
        data |= buffer[ i ];
        // since buffer is UBYTE *, i is addressing BYTEs, too
        if (( i & 6 ) == 6 ) {

          // Triggered every 4 bytes with the i += 2 above!
          WriteReg32( card, AMIGUS_WT_DATA_32BIT, data );
          data = 0;
        }
      }
      positionB += ( available >> 1 );

    } else {

      // 16bit STEREO
      UWORD * buffer = wav.wav_Buffer; // 1 Sample = 1 UWORD
      ULONG data;

      // Channel A
      WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, positionA );
      data = 0;
      for ( i = 0; i < ( available >> 1 ); i += 2 ) {

        data = ( data << 16 );
        data |= buffer[ i ];
        // since buffer is UWORD *, i is addressing WORDs, too
        if ( i & 2 ) {

          // Triggered every 4 bytes with the i += 2 above!
          WriteReg32( card, AMIGUS_WT_DATA_32BIT, data );
          data = 0;
        }
      }
      positionA += ( available >> 1 );

      // Channel B
      WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, positionB );
      data = 0;
      for ( i = 1; i < ( available >> 1 ); i += 2 ) {

        data = ( data << 16 );
        data |= buffer[ i ];
        // since buffer is UWORD *, i is addressing WORDs, too
        if ( i & 2 ) {

          // Triggered every 4 bytes with the i += 2 above!
          WriteReg32( card, AMIGUS_WT_DATA_32BIT, data );
          data = 0;
        }
      }
      positionB += ( available >> 1 );
    }
  }

  /*****************************************************************************
   * Starting playback finally... Easy, eh?
   ****************************************************************************/

  StartAmiGusWavetablePlayback( card,
                                wav.wav_Channels,
                                wav.wav_SampleRate,
                                wav.wav_SampleBits,
                                positionA - 4 );

  Printf( "Did you know? AmiGUS can play alone...\n" );

  CloseWav( &wav );

  /*****************************************************************************
   * Sorry for the ... hack.
   ****************************************************************************/
// Yes, this is awful, but keeps this demo code readable...
cleanup:

  if ( amigus ) {

    AmiGUS_FreeCard( amigus, AMIGUS_FLAG_WAVETABLE, mainTask );
  }
  if ( AmiGUS_Base ) {

    CloseLibrary( AmiGUS_Base );
  }

  return result;
}
