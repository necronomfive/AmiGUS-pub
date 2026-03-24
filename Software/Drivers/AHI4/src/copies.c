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

#include "amigus_ahi_sub.h"
#include "amigus_hardware.h"
#include "copies.h"

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

  WriteReg32( AmiGUS_AHI_Base->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, out );

  ( *bufferIndex ) += 2;
  return 4;
}

ASM( LONG ) PlaybackCopy16to16(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  ULONG addressIn = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG in = *(( ULONG * ) addressIn);
  ULONG out = in;

  WriteReg32( AmiGUS_AHI_Base->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, out );

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

  WriteReg32( AmiGUS_AHI_Base->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, out );

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

  WriteReg32( AmiGUS_AHI_Base->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, out );

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

  WriteReg32( AmiGUS_AHI_Base->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, outA );
  WriteReg32( AmiGUS_AHI_Base->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, outB );
  WriteReg32( AmiGUS_AHI_Base->agb_CardBase, AMIGUS_PCM_PLAY_FIFO_WRITE, outC );

  ( *bufferIndex ) += 4;
  return 12;
}

/* Private definition of the copy functions used for RECORDING finally. */

ASM( LONG ) RecordingCopy8Mto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  // AmiGUS 8Bit Mono => AHI 16Bit Stereo Non-HiFi
  ULONG in = ReadReg32( AmiGUS_AHI_Base->agb_CardBase,
                        AMIGUS_PCM_REC_FIFO_READ );
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
  return 4;
}

ASM( LONG ) RecordingCopy8Sto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  // AmiGUS 8Bit Stereo => AHI 16Bit Stereo Non-HiFi
  ULONG in = ReadReg32( AmiGUS_AHI_Base->agb_CardBase,
                        AMIGUS_PCM_REC_FIFO_READ );
  ULONG outA = (( in & 0xff000000 )       ) | ( ( in & 0x00FF0000 ) >> 8 );
  ULONG outB = (( in & 0x0000ff00 ) << 16 ) | ( ( in & 0x000000FF ) << 8 );
  ULONG addressOut = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  
  *( ULONG * ) addressOut = outA;
  addressOut += 4;

  *( ULONG * ) addressOut = outB;

  ( *bufferIndex ) += 2;
  return 4;
}

ASM( LONG ) RecordingCopy16Mto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  // AmiGUS 16Bit Mono => AHI 16Bit Stereo Non-HiFi
  ULONG in = ReadReg32( AmiGUS_AHI_Base->agb_CardBase,
                        AMIGUS_PCM_REC_FIFO_READ );
  ULONG outA = ( in & 0xffFF0000 ) | ( in >> 16 );
  ULONG outB = ( in & 0x0000ffFF ) | ( in << 16 );
  ULONG addressOut = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));

  *( ULONG * ) addressOut = outA;
  addressOut += 4;

  *( ULONG * ) addressOut = outB;
  
  ( *bufferIndex ) += 2;
  return 4;
}

ASM( LONG ) RecordingCopy16Sto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  // AmiGUS 16Bit Stereo => AHI 16Bit Stereo Non-HiFi
  ULONG in = ReadReg32( AmiGUS_AHI_Base->agb_CardBase,
                        AMIGUS_PCM_REC_FIFO_READ );
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
  ULONG inA = ReadReg32( AmiGUS_AHI_Base->agb_CardBase,
                         AMIGUS_PCM_REC_FIFO_READ );
  UWORD inB = ReadReg16( AmiGUS_AHI_Base->agb_CardBase,
                         AMIGUS_PCM_REC_FIFO_READ );
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
  return 6;
}

ASM( LONG ) RecordingCopy24Sto32S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex )) {

  // TODO: Replace placeholder after test passing!
  static UBYTE i = 0;
  // AmiGUS 16Bit Stereo => AHI 32Bit Stereo HiFi
  ULONG inA = ReadReg32( AmiGUS_AHI_Base->agb_CardBase,
                         AMIGUS_PCM_REC_FIFO_READ );
  ULONG inB = ReadReg32( AmiGUS_AHI_Base->agb_CardBase,
                         AMIGUS_PCM_REC_FIFO_READ );
  ULONG inC = ReadReg32( AmiGUS_AHI_Base->agb_CardBase,
                         AMIGUS_PCM_REC_FIFO_READ );
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
  return 12;
}