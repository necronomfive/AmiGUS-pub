/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <proto/exec.h>
#include <proto/utility.h>

#include "amigus_ahi_modes.h"
#include "amigus_ahi_sub.h"
#include "amigus_hardware.h"
#include "buffers.h"
#include "debug.h"
#include "SDI_compiler.h"

#define MILLIS_PER_SECOND   1000
#define DIVISOR_4MS         ( MILLIS_PER_SECOND / 250 )
#define DIVISOR_10MS        ( MILLIS_PER_SECOND / 100 )
#define DIVISOR_25MS        ( MILLIS_PER_SECOND / 40 )
#define DIVISOR_40MS        ( MILLIS_PER_SECOND / 25 )
#define DIVISOR_100MS       ( MILLIS_PER_SECOND / 10 )
#define DIVISOR_250MS       ( MILLIS_PER_SECOND / 4 )
#define RECORDING_BUFFER_DIVISOR  DIVISOR_10MS   /* = 100 buffers per second */

/**
 * Function to return the greatest common denominator of two numbers.
 *
 * @param a The first number.
 * @param b The other number.
 *
 * @return The result.
 */
UWORD gcd( UWORD a, UWORD b ) {

  UWORD x;

  if ( b == 0 ) {

    return a;    
  }
  UDivMod32(a, b); // Not interested in result - just in reminder :D
  x = (UWORD)GET_REG(REG_D1);

  return gcd( b, x );
}

/** 
 * Function to return the least common multiple of two numbers.
 *
 * @param a The first number.
 * @param b The other number.
 *
 * @return The result.
 */
ULONG lcm( ULONG a, ULONG b ) {

  ULONG c = gcd( a, b );
  ULONG d = UDivMod32( a, c );

  return UMult32( d, b );
}

ULONG getBufferSize(
  LONG sampleRate,
  LONG secondFraction,
  UBYTE sampleSize,
  UBYTE multipleOf
) {

  const LONG fractionTarget = SDivMod32( sampleRate, secondFraction );
  const LONG fractionSize = SMult32( fractionTarget, sampleSize );
  const LONG finalMultipleOf = lcm( sampleSize, multipleOf );

  LONG remainder;
  UWORD result;

  SDivMod32( fractionSize, finalMultipleOf );
  remainder = GET_REG(REG_D1);
  result = (UWORD)( fractionSize - remainder );

  return result;
}

ULONG getBufferBytes(
  LONG sampleRate,
  UBYTE sampleSize,
  UBYTE multipleOf,
  UBYTE isStereo,
  UBYTE isRealtime ) {

  const UBYTE finalSampleSize = isStereo ? sampleSize << 1 : sampleSize;
  const LONG divisor = isRealtime ? DIVISOR_10MS : DIVISOR_25MS;

  return getBufferSize( sampleRate, divisor, finalSampleSize, multipleOf );
}

UWORD getBufferSamples(
  UWORD bufferBytes,
  UBYTE sampleSize,
  UBYTE isStereo ) {

  const UBYTE finalSampleSize = isStereo ? sampleSize << 1 : sampleSize;
  const UWORD result = SDivMod32( bufferBytes, finalSampleSize );

  return result;
}

ULONG getRecordingBufferSize( LONG sampleRate ) {

  UBYTE modeOffset = AmiGUS_AHI_Base->agb_AhiModeOffset;
  struct RecordingProperties * mode = &RecordingPropertiesById[ modeOffset ];
  const LONG recordingDivisor = RECORDING_BUFFER_DIVISOR;
  const UBYTE shift = mode->rp_AhiSampleShift;
  const UBYTE sampleSize = ( 1 << shift );
  const UBYTE multipleOf = mode->rp_AhiBufferMultiples;
  const ULONG byteSize = getBufferSize(
    sampleRate,
    recordingDivisor,
    sampleSize,
    multipleOf );

  LOG_D(( "V: Calculated %ld BYTEs recording buffer for %ldHz, 1/%ld second, "
          "%ld BYTEs per AHI sample and enforcing %ld multiples.\n",
          byteSize,
          sampleRate,
          recordingDivisor,
          sampleSize,
          multipleOf ));

  return byteSize;
}

// TRUE = failure
BOOL CreatePlaybackBuffers( ULONG bufferSize ) {

  struct AmiGUSPcmPlayback * playback = &AmiGUS_AHI_Base->agb_Playback;
  const LONG longSize = bufferSize >> 2; /* Buffers are ticking in LONGs! */

  if ( playback->agpp_Buffer[0] ) {

    LOG_D(("D: Playback buffers already exist!\n"));
    return FALSE;
  }

  /* BufferSize is in BYTEs! */
  LOG_D(( "D: Allocating playback buffers a %ld BYTEs\n", bufferSize ));
  playback->agpp_Buffer[0] = (ULONG *)
    AllocVec( bufferSize, MEMF_FAST | MEMF_CLEAR );
  if ( !playback->agpp_Buffer[0] ) {

    LOG_E(("E: Could not allocate FAST RAM for playback buffer 0!\n"));
    return TRUE;
  }
  playback->agpp_Buffer[1] = (ULONG *)
    AllocVec( bufferSize, MEMF_FAST | MEMF_CLEAR );
  if ( !playback->agpp_Buffer[1] ) {

    LOG_E(("E: Could not allocate FAST RAM for playback buffer 1!\n"));
    return TRUE;
  }

  playback->agpp_BufferMax[ 0 ] = longSize;
  playback->agpp_BufferMax[ 1 ] = longSize;
    /* All buffers are created empty - back to initial state! */
  playback->agpp_BufferIndex[ 0 ] = longSize;
  playback->agpp_BufferIndex[ 1 ] = longSize;
  playback->agpp_CurrentBuffer = 0;

  LOG_I(( "I: Mix / copy up to %ld WORDs from AHI per pass\n",
          longSize << 1 ));

  LOG_D(("D: All playback buffers created\n"));
  return FALSE;
}

VOID DestroyPlaybackBuffers( VOID ) {

  struct AmiGUSPcmPlayback * playback = &AmiGUS_AHI_Base->agb_Playback;

  if ( playback->agpp_Buffer[0] ) {

    FreeVec( playback->agpp_Buffer[0] );
    playback->agpp_Buffer[0] = NULL;
    playback->agpp_BufferIndex[0] = 0;
    LOG_D(("D: Free`ed playback buffer 0!\n"));
  }
  if ( playback->agpp_Buffer[1] ) {

    FreeVec( playback->agpp_Buffer[1] );
    playback->agpp_Buffer[1] = NULL;
    playback->agpp_BufferIndex[1] = 0;
    LOG_D(("D: Free`ed playback buffer 1!\n"));
  }
  LOG_D(("D: All playback buffers free`ed\n"));
}

BOOL CreateRecordingBuffers( VOID ) {

  struct AmiGUSPcmRecording * recording = &AmiGUS_AHI_Base->agb_Recording;
  const LONG sampleRate =
    AmiGUSSampleRates[ AmiGUS_AHI_Base->agb_HwSampleRateId ];
  ULONG byteSize;
  
  if ( recording->agpr_Buffer[0] ) {

    LOG_D(("D: Recording buffers already exist!\n"));
    return FALSE;
  }
  byteSize = getRecordingBufferSize( sampleRate );
  LOG_D(( "D: Allocating %ld BYTEs recording buffer for %ldHz.\n",
          byteSize,
          sampleRate ));
  recording->agpr_Buffer[0] = (ULONG *)
    AllocVec( byteSize, MEMF_FAST | MEMF_CLEAR );
  if ( !recording->agpr_Buffer[0] ) {

    LOG_E(("E: Could not allocate FAST RAM for recording buffer 0!\n"));
    return TRUE;
  }
  recording->agpr_Buffer[1] = (ULONG *)
    AllocVec( byteSize, MEMF_FAST | MEMF_CLEAR );
  if ( !recording->agpr_Buffer[1] ) {

    LOG_E(("E: Could not allocate FAST RAM for recording buffer 1!\n"));
    return TRUE;
  }

  recording->agpr_BufferMax[ 0 ] = byteSize >> 2;
  recording->agpr_BufferMax[ 1 ] = byteSize >> 2;
    /* All buffers are created empty - back to initial state! */
  recording->agpr_BufferIndex[ 0 ] = 0;
  recording->agpr_BufferIndex[ 1 ] = 0;
  recording->agpr_CurrentBuffer = 0;

  LOG_I(( "I: Record / copy up to %ld WORDs to AHI per pass\n",
          byteSize >> 1 ));

  LOG_D(("D: All recording buffers created\n"));
  return FALSE;
}

VOID DestroyRecordingBuffers( VOID ) {

  struct AmiGUSPcmRecording * recording = &AmiGUS_AHI_Base->agb_Recording;

  if ( recording->agpr_Buffer[0] ) {

    FreeVec( recording->agpr_Buffer[0] );
    recording->agpr_Buffer[0] = NULL;
    recording->agpr_BufferIndex[0] = 0;
    LOG_D(("D: Free`ed recording buffer 0!\n"));
  }
  if ( recording->agpr_Buffer[1] ) {

    FreeVec( recording->agpr_Buffer[1] );
    recording->agpr_Buffer[1] = NULL;
    recording->agpr_BufferIndex[1] = 0;
    LOG_D(("D: Free`ed recording buffer 1!\n"));
  }
  LOG_D(("D: All recording buffers free`ed\n"));
}

ULONG AlignByteSizeForSamples( ULONG ahiBufferSamples ) {

  UBYTE modeOffset = AmiGUS_AHI_Base->agb_AhiModeOffset;
  struct PlaybackProperties * mode = &PlaybackPropertiesById[ modeOffset ];
  const ULONG mask = mode->pp_AhiBufferMask;
  const UBYTE shift = mode->pp_AhiSampleShift;
  ULONG aligned = ahiBufferSamples;

  aligned <<= shift;    /* now: in BYTEs! */
  if ( ( ~ mask ) & aligned ) {

    LOG_W(( "W: Correcting alignment of (%ld samples) %ld BYTEs to %ld.\n",
            ahiBufferSamples,
            aligned,
            mask & aligned ));
    aligned &= mask;
  }

  return aligned;
}
