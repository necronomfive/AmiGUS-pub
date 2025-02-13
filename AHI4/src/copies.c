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

#include "amigus_ahi_sub.h"
#include "amigus_hardware.h"
#include "copies.h"

/*
 * Privately declaring the copy functions here - the external interface 
 * are the PlaybackCopyFunctionById / RecordingCopyFunctionById lookup 
 * tables. In C++ we would have a nice virtual interface, but this is C -
 * function pointers for the win!
 */

/**
 * Reads 2 LONGs, a, and b,
 * writes 1 LONG, 
 *   ( a & 0xff000000 )       |
 *   ( a & 0x0000ff00 ) <<  8 | 
 *   ( b & 0xff000000 ) >> 16 |
 *   ( b & 0x0000ff00 ) >>  8.
 * Used for 8bit non-HiFi modes.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 2 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM(LONG) PlaybackCopy16to8(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * Reads 1 LONG, writes the same 1 LONG.
 * Used for 16bit non-HiFi modes.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 1 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM(LONG) PlaybackCopy16to16(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * Reads 4 LONGs, a, b, c, and d,
 * writes 1 LONG, 
 *   ( a & 0xff000000 )       |
 *   ( b & 0xff000000 ) >>  8 | 
 *   ( c & 0xff000000 ) >> 16 |
 *   ( d & 0xff000000 ) >> 24.
 * Used for 8bit HiFi modes.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 2 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM(LONG) PlaybackCopy32to8(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * Reads 2 LONGs, a, and b,
 * writes 1 LONG, a & 0xffFF0000 | b >> 16.
 * Used for 16bit HiFi modes.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 2 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM(LONG) PlaybackCopy32to16(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * Reads 4 LONGs, a, b, c, and d,
 * writes 3 LONGs,
 * 1:         a & 0xffFFff00 | b >> 24, 
 * 2:  (b << 8) & 0xffFF0000 | c >> 16,
 * 3: (c << 16) & 0xFF000000 | d >> 8
 * Used for 24bit HiFi modes.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 12.
 */
ASM(LONG) PlaybackCopy32to24(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * Reads 1 LONG aka 4 samples in 8Bit Mono from AmiGUS
 * and prepares it for AHI 16Bit Stereo Non-HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM( LONG ) RecordingCopy8Mto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

/**
 * Reads 1 LONG aka 2 samples in 8Bit Stereo from AmiGUS
 * and prepares it for AHI 16Bit Stereo Non-HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM( LONG ) RecordingCopy8Sto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

/**
 * Reads 1 LONG aka 2 samples in 16Bit Mono from AmiGUS
 * and prepares it for AHI 16Bit Stereo Non-HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM( LONG ) RecordingCopy16Mto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

/**
 * Reads 1 LONG aka 1 sample in 16Bit Stereo from AmiGUS
 * and prepares it for AHI 16Bit Stereo Non-HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM( LONG ) RecordingCopy16Sto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

/**
 * Reads 3 LONG aka 4 samples in 24Bit Mono from AmiGUS
 * and prepares it for AHI 32Bit Stereo HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 8 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 32.
 */
ASM( LONG ) RecordingCopy24Mto32S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

/**
 * Reads 6 LONG aka 8 samples in 24Bit Stereo from AmiGUS
 * and prepares it for AHI 32Bit Stereo HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 8 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 32.
 */
ASM( LONG ) RecordingCopy24Sto32S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

/* Public interface as announced above */

const CopyFunctionType PlaybackCopyFunctionById[] = {
  &PlaybackCopy16to8,
  &PlaybackCopy16to16,
  &PlaybackCopy32to8,
  &PlaybackCopy32to16,
  &PlaybackCopy32to24
};

const CopyFunctionType RecordingCopyFunctionById[] = {
  &RecordingCopy8Mto16S,
  &RecordingCopy8Sto16S,
  &RecordingCopy16Mto16S,
  &RecordingCopy16Sto16S,
  &RecordingCopy24Mto32S,
  &RecordingCopy24Sto32S
};

const UBYTE RecordingSampleTypeById[] = {
  AHIST_S16S,
  AHIST_S16S,
  AHIST_S16S,
  AHIST_S16S,
  AHIST_S32S,
  AHIST_S32S
};

const UBYTE RecordingSampleAlignmentById[] = {
  8,  // RecordingCopy8Mto16S  : 4x1 BYTE in, 8x2 BYTE out
  4,  // RecordingCopy8Sto16S  : 4x1 BYTE in, 4x2 BYTE out
  8,  // RecordingCopy16Mto16S : 2x2 BYTE in, 4x2 BYTE out
  4,  // RecordingCopy16Sto16S : 2x2 BYTE in, 2x2 BYTE out
  32, // RecordingCopy24Mto32S : 4x3 BYTE in, 8x4 BYTE out
  16  // RecordingCopy24Sto32S : 4x3 BYTE in, 4x4 BYTE out
};

/* Private definition of the copy functions used for PLAYBACK first. */

ASM( LONG ) PlaybackCopy16to8(
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

ASM( LONG ) PlaybackCopy16to16(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  ULONG addressIn = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG in = *(( ULONG * ) addressIn);
  ULONG out = in;

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, out );

  ( *bufferIndex ) += 1;
  return 4;
}

ASM( LONG ) PlaybackCopy32to8(
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

ASM( LONG ) PlaybackCopy32to16(
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

ASM( LONG ) PlaybackCopy32to24(
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

/* Private definition of the copy functions used for RECORDING finally. */

ASM( LONG ) RecordingCopy8Mto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  // AmiGUS 8Bit Mono => AHI 16Bit Stereo Non-HiFi
  ULONG in = ReadReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_REC_FIFO_READ );
  ULONG outA = (( in & 0xff000000 )       ) | ( ( in & 0xff000000 ) >> 16 );
  ULONG outB = (( in & 0x00FF0000 ) <<  8 ) | ( ( in & 0x00FF0000 ) >>  8 );
  ULONG outC = (( in & 0x0000ff00 ) << 16 ) | ( ( in & 0x0000ff00 )       );
  ULONG outD = (( in & 0x000000FF ) << 24 ) | ( ( in & 0x000000FF ) <<  8 );
  ULONG addressOut = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  
  *( ULONG * ) addressOut = outA;
  addressOut += 4;

  *( ULONG * ) addressOut = outB;
  addressOut += 4;

  *( ULONG * ) addressOut = outC;
  addressOut += 4;

  *( ULONG * ) addressOut = outD;

  ( *bufferIndex ) += 4;
  return 16;
}

ASM( LONG ) RecordingCopy8Sto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  // AmiGUS 8Bit Stereo => AHI 16Bit Stereo Non-HiFi
  ULONG in = ReadReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_REC_FIFO_READ );
  ULONG outA = (( in & 0xff000000 )       ) | ( ( in & 0x00FF0000 ) >> 8 );
  ULONG outB = (( in & 0x0000ff00 ) << 16 ) | ( ( in & 0x000000FF ) << 8 );
  ULONG addressOut = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  
  *( ULONG * ) addressOut = outA;
  addressOut += 4;

  *( ULONG * ) addressOut = outB;

  ( *bufferIndex ) += 2;
  return 8;
}

ASM( LONG ) RecordingCopy16Mto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  // AmiGUS 16Bit Mono => AHI 16Bit Stereo Non-HiFi
  ULONG in = ReadReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_REC_FIFO_READ );
  ULONG outA = ( in & 0xffFF0000 ) | ( in >> 16 );
  ULONG outB = ( in & 0x0000ffFF ) | ( in << 16 );
  ULONG addressOut = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));

  *( ULONG * ) addressOut = outA;
  addressOut += 4;

  *( ULONG * ) addressOut = outB;
  
  ( *bufferIndex ) += 2;
  return 8;
}

ASM( LONG ) RecordingCopy16Sto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  // AmiGUS 16Bit Stereo => AHI 16Bit Stereo Non-HiFi
  ULONG in = ReadReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_REC_FIFO_READ );
  ULONG out = in;
  ULONG addressOut = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  
  *( ULONG * ) addressOut = out;

  ( *bufferIndex ) += 1;
  return 4;
}

ASM( LONG ) RecordingCopy24Mto32S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  // TODO: Replace placeholder after test passing!
  static UBYTE i = 0;
  // AmiGUS 16Bit Mono => AHI 32Bit Stereo HiFi
  ULONG inA = ReadReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_REC_FIFO_READ );
  UWORD inB = ReadReg16( AmiGUSBase->agb_CardBase, AMIGUS_PCM_REC_FIFO_READ );
  ULONG outA = (( inA & 0xffFF0000 )       ) | (( ++i ) << 8 );
  ULONG outB = (( inA & 0x0000ffFF ) << 16 ) | (( ++i ) << 8 );
  ULONG outC = (( inB & 0x0000ffFF ) << 16 ) | (( ++i ) << 8 );
  ULONG addressOut = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));

  *( ULONG * ) addressOut = outA;
  addressOut += 4;
  *( ULONG * ) addressOut = outA;
  addressOut += 4;

  *( ULONG * ) addressOut = outB;
  addressOut += 4;
  *( ULONG * ) addressOut = outB;
  addressOut += 4;

  *( ULONG * ) addressOut = outC;
  addressOut += 4;
  *( ULONG * ) addressOut = outC;
  
  ( *bufferIndex ) += 6;
  return 24;
}

ASM( LONG ) RecordingCopy24Sto32S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  // TODO: Replace placeholder after test passing!
  static UBYTE i = 0;
  // AmiGUS 16Bit Stereo => AHI 32Bit Stereo HiFi
  ULONG inA = ReadReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_REC_FIFO_READ );
  ULONG inB = ReadReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_REC_FIFO_READ );
  ULONG inC = ReadReg32( AmiGUSBase->agb_CardBase, AMIGUS_PCM_REC_FIFO_READ );
  ULONG outA = (( inA & 0xffFF0000 )       ) | (( ++i ) << 8 );
  ULONG outB = (( inA & 0x0000ffFF ) << 16 ) | (( ++i ) << 8 );
  ULONG outC = (( inB & 0xffFF0000 )       ) | (( ++i ) << 8 );
  ULONG outD = (( inB & 0x0000ffFF ) << 16 ) | (( ++i ) << 8 );
  ULONG outE = (( inC & 0xffFF0000 )       ) | (( ++i ) << 8 );
  ULONG outF = (( inC & 0x0000ffFF ) << 16 ) | (( ++i ) << 8 );
  ULONG addressOut = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));

  *( ULONG * ) addressOut = outA;
  addressOut += 4;
  *( ULONG * ) addressOut = outB;
  addressOut += 4;

  *( ULONG * ) addressOut = outC;
  addressOut += 4;
  *( ULONG * ) addressOut = outD;
  addressOut += 4;

  *( ULONG * ) addressOut = outE;
  addressOut += 4;
  *( ULONG * ) addressOut = outF;
  
  ( *bufferIndex ) += 6;
  return 24;
}