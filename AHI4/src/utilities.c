#include <proto/utility.h>

#include "amigus_hardware.h"
#include "amigus_private.h" // WriteReg32
#include "utilities.h"
#include "SDI_compiler.h"

#define MILLIS_PER_SECOND      1000
#define DIVISOR_10MS           ( MILLIS_PER_SECOND / 10 )
#define DIVISOR_25MS           ( MILLIS_PER_SECOND / 40 )

UWORD gcd( UWORD a, UWORD b ) {

  UWORD x;

  if ( b == 0 ) {

    return a;    
  }
  UDivMod32(a, b); // Not interested in result - just in reminder :D
  x = (UWORD)GET_REG(REG_D1);

  return gcd( b, x );
}

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

LONG getNextSampleData( VOID ) {

  return 0;
}

const ULONG CopyFunctionRequirementById[] = {
  0xffFFffF8, /* Needs 2 LONGs to work properly, */
  0xffFFffFC, /*       1 LONG,                   */
  0xffFFffF0, /*       4 LONGs,                  */
  0xffFFffF8, /*       2 LONGs                   */
  0xffFFffF0  /*       4 LONGs                   */
};

ULONG alignBufferSamples( ULONG ahiBuffSamples ) {

  ULONG index = AmiGUSBase->agb_CopyFunctionId;
  ULONG mask = CopyFunctionRequirementById[ index ];
  UBYTE shift = AmiGUSBase->agb_AhiSampleShift;
  ULONG aligned = ahiBuffSamples;

  aligned &= ( mask >> shift );

  return aligned;
}

CopyFunctionType CopyFunctionById[] = {
  &Copy16to8,
  &Copy16to16,
  &Copy32to8,
  &Copy32to16,
  &Copy32to24
};

ASM(LONG) Copy16to8(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) ) {

  ULONG addressInA = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG inA = *(( ULONG * ) addressInA);
  ULONG addressInB = addressInA + sizeof( ULONG );
  ULONG inB = *(( ULONG * ) addressInB);
  ULONG out = ( inA & 0xFF000000 )       |
              ( inA & 0x0000FF00 ) <<  8 |
              ( inB & 0xFF000000 ) >> 16 |
              ( inB & 0x0000FF00 ) >>  8;

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, out );

  ( *bufferIndex ) += 2;
  return 4;
}

ASM(LONG) Copy16to16(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) ) {

  ULONG addressIn = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG in = *(( ULONG * ) addressIn);
  ULONG out = in;

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, out );

  ( *bufferIndex ) += 1;
  return 4;
}

ASM(LONG) Copy32to8(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) ) {

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

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, out );

  ( *bufferIndex ) += 4;
  return 4;
}

ASM(LONG) Copy32to16(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) ) {

  ULONG addressInA = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG inA = *(( ULONG * ) addressInA);
  ULONG addressInB = addressInA + sizeof( ULONG );
  ULONG inB = *(( ULONG * ) addressInB);
  ULONG out = ( inA & 0xFFff0000 ) | ( inB >> 16 );

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, out );

  ( *bufferIndex ) += 2;
  return 4;
}

ASM(LONG) Copy32to24(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) ) {

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

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, outA );
  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, outB );
  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, outC );

  ( *bufferIndex ) += 4;
  return 12;
}
