#include "amigus_hardware.h"
#include "amigus_private.h" // WriteReg32
#include "copy_functions.h"

ASM(LONG) SAVEDS PlainLongCopy(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex)
) {

  ULONG addressIn = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG in = *(( ULONG * ) addressIn);
  ULONG out = in;

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, out );

  ( *bufferIndex ) += 1;
  return 4;
}

ASM(LONG) SAVEDS Shift16LongCopy(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex)
) {

  ULONG addressInA = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG inA = *(( ULONG * ) addressInA);
  ULONG addressInB = addressInA + sizeof( ULONG );
  ULONG inB = *(( ULONG * ) addressInB);
  ULONG out = ( inA & 0xFFff0000 ) | ( inB >> 16 );

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, out );

  ( *bufferIndex ) += 2;
  return 4;
}

ASM(LONG) SAVEDS Merge24LongCopy(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex)
) {

  ULONG addressInA = ((( ULONG ) bufferBase ) + ( ( *bufferIndex ) << 2 ));
  ULONG inA = *(( ULONG * ) addressInA);
  ULONG addressInB = addressInA + sizeof( ULONG );
  ULONG inB = *(( ULONG * ) addressInB);
  ULONG addressInC = addressInB + sizeof( ULONG );
  ULONG inC = *(( ULONG * ) addressInC);
  ULONG addressInD = addressInC + sizeof( ULONG );
  ULONG inD = *(( ULONG * ) addressInD);
  ULONG outA = (   inA         & 0xFFffFF00 ) | ( inB >> 24 );
  ULONG outB = ( ( inB <<  8 ) & 0xFFff0000 ) | ( inC >> 16 );
  ULONG outC = ( ( inC << 16 ) & 0xFF000000 ) | ( inD >>  8 );

  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, outA );
  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, outB );
  WriteReg32( AmiGUSBase->agb_CardBase, AMIGUS_MAIN_FIFO_WRITE, outC );

  ( *bufferIndex ) += 4;
  return 12;
}
