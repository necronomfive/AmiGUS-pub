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

#include "AIFF.h"
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
                                   UWORD flags,
                                   ULONG count ) {

  ULONG middle = 1 << 24;
  // ULONG end = 1 << 25;
  double maxFreq = 0x40000000;
	double maxRate = 192000;
	ULONG frequency = (( double ) sampleRate ) * maxFreq / maxRate;
  UWORD channel0VolumeRight = ( 2 == channels ) ? 0 : 0xFFFF;

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
              | flags );

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
              | flags );
}

/**
 * Shuffles and copys the sample data into AmiGUS Wavetable RAM using
 * the AutoIncrement feature of the target addresses for that.
 *
 * Yes, we need to set / reset these once per block, because the function
 * contains the copy and re-sort function for all bytes from the AIFF/WAV.
 * Why?
 * - AmiGUS WaveTable needs 1 stream of consecutive samples PER VOICE,
 *   while WAV comes with interleaved samples, hence the inefficient
 *   copy LONG by LONG into AmiGUS memory
 *
 * @param card Base address of the AmiGUS Wavetable part.
 * @param data Source address of the sample data blob.
 * @param length Number of BYTEs in the sample data blob.
 * @param channels 1 for MONO, 2 for STEREO.
 * @param bits Bits per sample, 8 or 16.
 */
VOID ShuffleCopySampleData(
  APTR card,
  APTR data,
  LONG length,
  UWORD channels,
  UWORD bits ) {

  // Remember: these are static, making them kind of global, 
  // while their initial values are set only once...
  static ULONG positionA = 0;
  static ULONG positionB = 1 << 24;

  if ( 1 == channels ) {

    // 8bit or 16bit MONO - No need to fiddle around, can just be coppied

    LONG i;
    ULONG * buffer = data;

    WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, positionA );
    for ( i = 0; i < ( length >> 2 ); ++i ) {

      WriteReg32( card, AMIGUS_WT_DATA_32BIT, buffer[ i ]);
    }
    positionA += length;
    // Could also rely on auto-increment as it is only one channel,
    // but we love symmetric code, don't we?

  } else if ( 8 == bits ) {

    // 8bit STEREO - resort bytes from block into channels

    LONG i;
    UBYTE * buffer = data; // 1 Sample = 1 UBYTE
    ULONG temp;

    // Channel A
    WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, positionA );
    temp = 0;
    for ( i = 0; i < length; i += 2 ) {

      temp = ( temp << 8 );
      temp |= buffer[ i ];
      // since buffer is UBYTE *, i is addressing BYTEs, too
      if (( i & 6 ) == 6 ) {

        // Triggered every 4 bytes with the i += 2 above!
        WriteReg32( card, AMIGUS_WT_DATA_32BIT, temp );
        temp = 0;
      }
    }
    positionA += ( length >> 1 );

    // Channel B
    WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, positionB );
    temp = 0;
    for ( i = 1; i < length; i += 2 ) {

      temp = ( temp << 8 );
      temp |= buffer[ i ];
      // since buffer is UBYTE *, i is addressing BYTEs, too
      if (( i & 6 ) == 6 ) {

        // Triggered every 4 bytes with the i += 2 above!
        WriteReg32( card, AMIGUS_WT_DATA_32BIT, temp );
        temp = 0;
      }
    }
    positionB += ( length >> 1 );

  } else if ( 16 == bits ) {

    // 16bit STEREO

    LONG i;
    UWORD * buffer = data; // 1 Sample = 1 UWORD
    ULONG temp;

    // Channel A
    WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, positionA );
    temp = 0;
    for ( i = 0; i < ( length >> 1 ); i += 2 ) {

      temp = ( temp << 16 );
      temp |= buffer[ i ];
      // since buffer is UWORD *, i is addressing WORDs, too
      if ( i & 2 ) {

        // Triggered every 4 bytes with the i += 2 above!
        WriteReg32( card, AMIGUS_WT_DATA_32BIT, temp );
        temp = 0;
      }
    }
    positionA += ( length >> 1 );

    // Channel B
    WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, positionB );
    temp = 0;
    for ( i = 1; i < ( length >> 1 ); i += 2 ) {

      temp = ( temp << 16 );
      temp |= buffer[ i ];
      // since buffer is UWORD *, i is addressing WORDs, too
      if ( i & 2 ) {

        // Triggered every 4 bytes with the i += 2 above!
        WriteReg32( card, AMIGUS_WT_DATA_32BIT, temp );
        temp = 0;
      }
    }
    positionB += ( length >> 1 );
  }
}

LONG LoadAiff( struct aiff * aiff, STRPTR filename ) {

  LONG result = OpenAiff( aiff, filename);
  if ( result ) {

    Printf( "Cannot read AIFF file, reason %ld.\n", result );
    return result;
  }
  if (( 1 << 25 ) < aiff->aiff_DataSize ) {

    Printf( "File too big, max %ld but is %ld bytes.\n",
            ( 1 << 25 ),
            aiff->aiff_DataSize );
    return 35;
  }

  Printf( "Found AIFF with %ldbit, %ldHz, %ld channels, %ld total bytes.\n",
          aiff->aiff_SampleBits,
          aiff->aiff_SampleRate,
          aiff->aiff_Channels,
          aiff->aiff_DataSize );

  return result;
}

LONG LoadWav( struct wav * wav, STRPTR filename ) {

  LONG result = OpenWav( wav, filename);
  if ( result ) {

    Printf( "Cannot read WAV file, reason %ld.\n", result );
    return result;
  }
  if (( 1 << 25 ) < wav->wav_DataSize ) {

    Printf( "File too big, max %ld but is %ld bytes.\n",
            ( 1 << 25 ),
            wav->wav_DataSize );
    return 35;
  }

  Printf( "Found WAV with %ldbit, %ldHz, %ld channels, %ld total bytes.\n",
          wav->wav_SampleBits,
          wav->wav_SampleRate,
          wav->wav_Channels,
          wav->wav_DataSize );

  return result;
}

VOID PlayAiff( APTR card, struct aiff * aiff ) {

  LONG available;
  UWORD flags;

  while ( available = ReadAiffChunkBE( aiff )) {

    // No unsigned -> signed conversion for 8bit in AIFF - ;)
    
    ShuffleCopySampleData( card,
                           aiff->aiff_Buffer,
                           available,
                           aiff->aiff_Channels,
                           aiff->aiff_SampleBits );
  }

  /*****************************************************************************
   * Starting playback finally... Easy, eh?
   ****************************************************************************/
  flags = AMIGUS_WT_F_INTERPOLATE;
  if ( 16 == aiff->aiff_SampleBits ) {

    flags |= AMIGUS_WT_F_RESOLUTION_16BIT;
  }
 
  StartAmiGusWavetablePlayback( card,
                                aiff->aiff_Channels,
                                aiff->aiff_SampleRate,
                                flags,
                                aiff->aiff_DataSize );
}

VOID PlayWav( APTR card, struct wav * wav ) {

  LONG available;
  UWORD flags;

  while ( available = ReadWavChunkLE( wav )) {

    if ( 8 == wav->wav_SampleBits ) {

      /*
       * unsigned -> signed conversion for the next block of data!
       * - AmiGUS WaveTable is always signed,
       *   WAV 8bit is unsigned, 16 bit is signed.
       */

      UBYTE * buffer = wav->wav_Buffer;
      LONG i;

      for ( i = 0; i < available; ++i ) {
        // Fix up signed vs unsigned for 8bit only

        buffer[ i ] -= 128;
      }
    }

    ShuffleCopySampleData( card,
                           wav->wav_Buffer,
                           available,
                           wav->wav_Channels,
                           wav->wav_SampleBits );
  }

  /*****************************************************************************
   * Starting playback finally... Easy, eh?
   ****************************************************************************/
  flags = AMIGUS_WT_F_INTERPOLATE;
  if ( 16 == wav->wav_SampleBits ) {

    flags |= AMIGUS_WT_F_LITTLE_ENDIAN | AMIGUS_WT_F_RESOLUTION_16BIT;
  }
 
  StartAmiGusWavetablePlayback( card,
                                wav->wav_Channels,
                                wav->wav_SampleRate,
                                flags,
                                wav->wav_DataSize );
}

/******************************************************************************
 * Entry point.
 *****************************************************************************/

int main( int argc, char **argv ) {

  struct Library * AmiGUS_Base = NULL;
  struct AmiGUS * amigus = NULL;
  struct Task * mainTask = FindTask( NULL );
  struct aiff aiff;
  struct wav wav;

  LONG result;

  Printf( "\n============================="
          "\n  AmiGUS PlayWAVetable V0.2  "
          "\n============================="
          "\n"
          "\nUsage: PlayWAVetable some.wav"
          "\nOR:    PlayWAVetable some.aiff"
          "\n"
          "\nKeep in mind: only AIFF/WAV files < 32MB supported!"
          "\n" );

  if ( argc < 2 ) {

    Printf( "No AIFF/WAV file name provided.\n" );
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

  /*****************************************************************************
   * Flushing AmiGUS WaveTable hardware.
   ****************************************************************************/

  InitAmiGus( amigus->agus_WavetableBase );

  /*****************************************************************************
   * Reading and parsing files... Kudos to wikipedia ;-)
   ****************************************************************************/

  result = LoadWav( &wav, argv[ 1 ]);
  if ( !result ) {

    Printf( "Playing WAV on card 0x%08lx (eb0000)\n",
            amigus->agus_WavetableBase );
    PlayWav( amigus->agus_WavetableBase, &wav );
    CloseWav( &wav );

  } else {

    result = LoadAiff( &aiff, argv[ 1 ]);
    if ( !result ) {

      Printf( "Playing AIFF on card 0x%08lx (eb0000)\n",
              amigus->agus_WavetableBase );
      PlayAiff( amigus->agus_WavetableBase, &aiff );
      CloseAiff( &aiff );

    } else{

      goto cleanup;
    }
  }

  Printf( "Did you know? AmiGUS can play alone...\n" );

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
