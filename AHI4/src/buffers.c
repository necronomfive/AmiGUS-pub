/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <proto/exec.h>
#include <proto/utility.h>

#include "amigus_hardware.h"
#include "amigus_private.h"
#include "buffers.h"
#include "debug.h"
#include "SDI_compiler.h"

#define MILLIS_PER_SECOND      1000
#define DIVISOR_10MS           ( MILLIS_PER_SECOND / 10 )
#define DIVISOR_25MS           ( MILLIS_PER_SECOND / 40 )

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

UWORD getBufferBytes(
  LONG sampleRate,
  UBYTE sampleSize,
  UBYTE multipleOf,
  UBYTE isStereo,
  UBYTE isRealtime ) {

  const UBYTE finalSampleSize = isStereo ? sampleSize << 1 : sampleSize;
  const LONG divisor = isRealtime ? DIVISOR_10MS : DIVISOR_25MS;
  const LONG sampleTarget = SDivMod32( sampleRate, divisor );
  const LONG bytesTarget = SMult32( sampleTarget, finalSampleSize );
  const LONG finalMultipleOf = lcm( finalSampleSize, multipleOf );
  LONG remainder;
  UWORD result;

  SDivMod32( bytesTarget, finalMultipleOf );
  remainder = GET_REG(REG_D1);
  result = (UWORD)( bytesTarget - remainder );

  return result;
}

UWORD getBufferSamples(
  UWORD bufferBytes,
  UBYTE sampleSize,
  UBYTE isStereo ) {

  const UBYTE finalSampleSize = isStereo ? sampleSize << 1 : sampleSize;
  const UWORD result = SDivMod32( bufferBytes, finalSampleSize );

  return result;
}

// TRUE = failure
BOOL CreatePlaybackBuffers( VOID ) {

  LONG longSize;
  struct AmiGUSPcmPlayback * playback = &AmiGUSBase->agb_Playback;

  if ( playback->agpp_Buffer[0] ) {

    LOG_D(("D: Playback buffers already exist!\n"));
    return FALSE;
  }

  /* BufferSize is in BYTEs! */
  playback->agpp_BufferSize = AmiGUSBase->agb_AudioCtrl->ahiac_BuffSize;
  LOG_D(( "D: Allocating playback buffers a %ld BYTEs\n",
          playback->agpp_BufferSize ));
  playback->agpp_Buffer[0] = (ULONG *)
    AllocMem( playback->agpp_BufferSize, MEMF_FAST | MEMF_CLEAR );
  if ( !playback->agpp_Buffer[0] ) {

    LOG_E(("E: Could not allocate FAST RAM for buffer 0!\n"));
    return TRUE;
  }
  playback->agpp_Buffer[1] = (ULONG *)
    AllocMem( playback->agpp_BufferSize, MEMF_FAST | MEMF_CLEAR );
  if ( !playback->agpp_Buffer[1] ) {

    LOG_E(("E: Could not allocate FAST RAM for buffer 1!\n"));
    return TRUE;
  }

  /* Buffers are ticking in LONGs! */
  longSize = playback->agpp_BufferSize >> 2;

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

VOID DestroyPlaybackBuffers(VOID) {

  struct AmiGUSPcmPlayback * playback = &AmiGUSBase->agb_Playback;

  if ( playback->agpp_Buffer[0] ) {

    FreeMem( playback->agpp_Buffer[0], playback->agpp_BufferSize );
    playback->agpp_Buffer[0] = NULL;
    playback->agpp_BufferIndex[0] = 0;
    LOG_D(("D: Free`ed buffer 0!\n"));
  }
  if ( playback->agpp_Buffer[1] ) {

    FreeMem( playback->agpp_Buffer[1], playback->agpp_BufferSize );
    playback->agpp_Buffer[1] = NULL;
    playback->agpp_BufferIndex[1] = 0;
    LOG_D(("D: Free`ed buffer 1!\n"));
  }
  playback->agpp_BufferSize = 0;
  LOG_D(("D: All playback buffers free`ed\n"));
}

BOOL CreateRecordingBuffers( VOID ) {

  return TRUE;
}

VOID DestroyRecordingBuffers( VOID ) {
}

/**
 * Alignment requirements of the copy functions encrypted into BYTE masks.
 * Order follows the same as CopyFunctionById[].
 */
const ULONG CopyFunctionRequirementById[] = {
  0xffFFffF8, /* Needs 2 LONGs to work properly, */
  0xffFFffFC, /*       1 LONG,                   */
  0xffFFffF0, /*       4 LONGs,                  */
  0xffFFffF8, /*       2 LONGs                   */
  0xffFFffF0  /*       4 LONGs                   */
};

ULONG AlignByteSizeForSamples( ULONG ahiBufferSamples ) {

  const ULONG index = AmiGUSBase->agb_Playback.agpp_CopyFunctionId;
  const ULONG mask = CopyFunctionRequirementById[ index ];
  const UBYTE shift = AmiGUSBase->agb_AhiSampleShift;
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

CopyFunctionType CopyFunctionById[] = {
  &Copy16to8,
  &Copy16to16,
  &Copy32to8,
  &Copy32to16,
  &Copy32to24
};

ASM( LONG ) Copy16to8(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  ULONG addressInA = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG inA = *(( ULONG * ) addressInA);
  ULONG addressInB = addressInA + sizeof( ULONG );
  ULONG inB = *(( ULONG * ) addressInB);
  ULONG out = ( inA & 0xFF000000 )       |
              ( inA & 0x0000FF00 ) <<  8 |
              ( inB & 0xFF000000 ) >> 16 |
              ( inB & 0x0000FF00 ) >>  8;

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, out );

  ( *bufferIndex ) += 2;
  return 4;
}

ASM( LONG ) Copy16to16(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  ULONG addressIn = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG in = *(( ULONG * ) addressIn);
  ULONG out = in;

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, out );

  ( *bufferIndex ) += 1;
  return 4;
}

ASM( LONG ) Copy32to8(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  ULONG addressInA = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG inA = *(( ULONG * ) addressInA);
  ULONG addressInB = addressInA + sizeof( ULONG );
  ULONG inB = *(( ULONG * ) addressInB);
  ULONG addressInC = addressInB + sizeof( ULONG );
  ULONG inC = *(( ULONG * ) addressInC);
  ULONG addressInD = addressInC + sizeof( ULONG );
  ULONG inD = *(( ULONG * ) addressInD);
  ULONG out = ( inA & 0xff000000 )       |
              ( inB & 0xff000000 ) >>  8 |
              ( inC & 0xff000000 ) >> 16 |
              ( inD & 0xff000000 ) >> 24;

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, out );

  ( *bufferIndex ) += 4;
  return 4;
}

ASM( LONG ) Copy32to16(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  ULONG addressInA = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG inA = *(( ULONG * ) addressInA);
  ULONG addressInB = addressInA + sizeof( ULONG );
  ULONG inB = *(( ULONG * ) addressInB);
  ULONG out = ( inA & 0xFFff0000 ) | ( inB >> 16 );

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, out );

  ( *bufferIndex ) += 2;
  return 4;
}

ASM( LONG ) Copy32to24(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  ULONG addressInA = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG inA = *(( ULONG * ) addressInA);
  ULONG addressInB = addressInA + sizeof( ULONG );
  ULONG inB = *(( ULONG * ) addressInB);
  ULONG addressInC = addressInB + sizeof( ULONG );
  ULONG inC = *(( ULONG * ) addressInC);
  ULONG addressInD = addressInC + sizeof( ULONG );
  ULONG inD = *(( ULONG * ) addressInD);
  ULONG outA = (   inA         & 0xffFFff00 ) | ( inB >> 24 );
  ULONG outB = ( ( inB <<  8 ) & 0xffFF0000 ) | ( inC >> 16 );
  ULONG outC = ( ( inC << 16 ) & 0xff000000 ) | ( inD >>  8 );

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, outA );
  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, outB );
  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, outC );

  ( *bufferIndex ) += 4;
  return 12;
}
