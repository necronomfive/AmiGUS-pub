/*
 * This file is part of the mhiAmiGUS.library.
 *
 * mhiAmiGUS.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>

#include "amigus_mhi.h"
#include "amigus_hardware.h"
#include "amigus_vs1063.h"

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/

struct AmiGUS_MHI        * AmiGUS_MHI_Base   = NULL;

const WORD AmiGUSDefaultEqualizer[ 9 ] = {
  0, /* +/- 0dB */   125, /* Hz */
  0, /* +/- 0dB */   500, /* Hz */
  0, /* +/- 0dB */  2000, /* Hz */
  0, /* +/- 0dB */  8000, /* Hz */
  0  /* +/- 0dB */
};

const UBYTE AmiGUSVolumeMapping[ 104 ] = {
  0xFE /*  0% */, 
  72 /*  1% */, 66 /*  2% */, 60 /*  3% */, 54 /*  4% */, 51 /*   5% */,
  48 /*  6% */, 45 /*  7% */, 42 /*  8% */, 41 /*  9% */, 39 /*  10% */,
  37 /* 11% */, 36 /* 12% */, 34 /* 13% */, 33 /* 14% */, 32 /*  15% */,
  31 /* 16% */, 30 /* 17% */, 30 /* 18% */, 29 /* 19% */, 28 /*  20% */,
  27 /* 21% */, 26 /* 22% */, 26 /* 23% */, 25 /* 24% */, 24 /*  25% */,
  23 /* 26% */, 23 /* 27% */, 22 /* 28% */, 21 /* 29% */, 21 /*  30% */,
  20 /* 31% */, 20 /* 32% */, 19 /* 33% */, 19 /* 34% */, 18 /*  35% */,
  18 /* 36% */, 17 /* 37% */, 17 /* 38% */, 16 /* 39% */, 16 /*  40% */,
  15 /* 41% */, 15 /* 42% */, 15 /* 43% */, 14 /* 44% */, 14 /*  45% */,
  13 /* 46% */, 13 /* 47% */, 12 /* 48% */, 12 /* 49% */, 12 /*  50% */,
  11 /* 51% */, 11 /* 52% */, 11 /* 53% */, 10 /* 54% */, 10 /*  55% */,
  10 /* 56% */,  9 /* 57% */,  9 /* 58% */,  8 /* 59% */,  8 /*  60% */,
   8 /* 61% */,  7 /* 62% */,  7 /* 63% */,  7 /* 64% */,  7 /*  65% */,
   6 /* 66% */,  6 /* 67% */,  6 /* 68% */,  6 /* 69% */,  6 /*  70% */,
   5 /* 71% */,  5 /* 72% */,  5 /* 73% */,  5 /* 74% */,  5 /*  75% */,
   4 /* 76% */,  4 /* 77% */,  4 /* 78% */,  4 /* 79% */,  4 /*  80% */,
   3 /* 81% */,  3 /* 82% */,  3 /* 83% */,  3 /* 84% */,  3 /*  85% */,
   2 /* 86% */,  2 /* 87% */,  2 /* 88% */,  2 /* 89% */,  2 /*  90% */,
   1 /* 91% */,  1 /* 92% */,  1 /* 93% */,  1 /* 94% */,  1 /*  95% */,
   0 /* 96% */,  0 /* 97% */,  0 /* 98% */,  0 /* 99% */,  0 /* 100% */,
  99          , 99          , 99 // padding back to LONGs
};

ULONG * NextWriteBuffer = NULL;
ULONG * WriteBuffer = NULL;

ULONG * NextReadBuffer = NULL;

ULONG ReadReg32( APTR card, ULONG offset ) {

  printf( "ReadReg32, expected 0x%08lx, is 0x%08lx\n",
          *NextReadBuffer, offset );
  assert( offset == *NextReadBuffer );

  ++NextReadBuffer;
  return *NextReadBuffer++;
}

UWORD ReadCodecSPI( APTR card, UWORD SPIregister ) {

  printf( "ReadCodecSPI, expected 0x%08lx, is 0x%08lx\n",
          ( ULONG ) *NextReadBuffer, ( ULONG ) SPIregister );
  assert( SPIregister == ( UWORD ) *NextReadBuffer );

  ++NextReadBuffer;
  return ( UWORD ) *NextReadBuffer++;
}

UWORD ReadVS1063Mem( APTR amiGUS, UWORD address ) {

  printf( "ReadVS1063Mem, expected 0x%08lx, is 0x%08lx\n",
          ( ULONG ) *NextReadBuffer, ( ULONG ) address );
  assert( address == ( UWORD ) *NextReadBuffer );

  ++NextReadBuffer;
  return ( UWORD ) *NextReadBuffer++;
}

#define WriteReg16_MARKER     0x10000000
#define WriteReg32_MARKER     0x20000000
#define WriteCodecSPI_MARKER  0x30000000
#define WriteVS1063Mem_MARKER 0x40000000

VOID WriteReg16( APTR card, ULONG offset, UWORD value ) {

  *NextWriteBuffer = WriteReg16_MARKER | offset;
  ++NextWriteBuffer;
  *NextWriteBuffer = 0x77770000 | value;
  ++NextWriteBuffer;
}

VOID WriteReg32( APTR card, ULONG offset, ULONG value ) {

  *NextWriteBuffer = WriteReg32_MARKER | offset;
  ++NextWriteBuffer;
  *NextWriteBuffer = value;
  ++NextWriteBuffer;
}

VOID WriteCodecSPI( APTR card, UWORD SPIregister, UWORD SPIvalue ) {

  *NextWriteBuffer = WriteCodecSPI_MARKER | SPIregister;
  ++NextWriteBuffer;
  *NextWriteBuffer = 0x77770000 | SPIvalue;
  ++NextWriteBuffer;
}

VOID WriteVS1063Mem( APTR amiGUS, UWORD address, UWORD value ) {

  *NextWriteBuffer = WriteVS1063Mem_MARKER | address;
  ++NextWriteBuffer;
  *NextWriteBuffer = 0x77770000 | value;
  ++NextWriteBuffer;
}

VOID SleepCodecTicks( ULONG ticks ) {}
 
 /******************************************************************************
  * Test functions:
  *****************************************************************************/

BOOL CheckWrites( ULONG * expectedWrites ) {

  ULONG * check = WriteBuffer;
  int i, j;

  printf( "Checking written values...\n");
  while ( check < NextWriteBuffer ) {

    ULONG count = *expectedWrites++;
    printf( "Checking %8lu writes of type 0x%08lx\n",
            count, ( LONG ) ( *expectedWrites & 0xf0000000 ));

    while ( count ) {
      for ( i = 0; i < 2 ; ++i ) {
        if ( *check != *expectedWrites ) {

          printf( "Written check, position: %ld, "
                  "actual: 0x%08lx, expected: 0x%08lx\n",
                  (( LONG ) check - ( LONG ) WriteBuffer ) >> 3,
                  *check,
                  *expectedWrites );
          printf( "Area around:\n");
          check -= 40;
          for ( j = 0; j < 80; ++j ) {

            printf( "0x%08lx ", *check++ );
            if ( 7 == j % 8 ) {

                printf( "\n" );
            }
          }
          return TRUE;
        }

        ++check;
        ++expectedWrites;
      }
      --count;
      if ( count ) {

        expectedWrites -= 2;
      }
    }
  }
  for ( i = 0; i < 2 ; ++i ) {
    if ( 0xffFFffFF != *expectedWrites++ ) {

      printf( "Less bytes written than expected!!!\n" );
      return TRUE;
    }
  }
  printf( "OK\n" );
  return FALSE;
}

BOOL testCancelPlaybackVanilla( VOID ) {

  BOOL result = FALSE;
  
  ULONG ReadBuffer[] = {
    VS1063_CODEC_SCI_HDAT1, 0x4d50,
    VS1063_CODEC_ADDRESS_END_FILL, 0x1234,
    AMIGUS_CODEC_FIFO_USAGE, 17,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    VS1063_CODEC_SCI_MODE, 0,
    VS1063_CODEC_SCI_MODE, 0,
#if defined(__SASC)
    VS1063_CODEC_SCI_HDAT0, 0x4242,
    VS1063_CODEC_SCI_HDAT1, 0x4242,
#elif defined(__VBCC__)
    VS1063_CODEC_SCI_HDAT1, 0x4242,
    VS1063_CODEC_SCI_HDAT0, 0x4242,
#endif
    0xffFFffFF, 0xffFFffFF
  };
  ULONG expectedWrites[] = {
    1,      WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_ENABLE,
    768,    WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,      WriteCodecSPI_MARKER | VS1063_CODEC_SCI_MODE,  0x77770000 | VS1063_CODEC_F_SM_CANCEL,
    8,      WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,      WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_DISABLE,
    0xffFFffFF, 0xffFFffFF
  };

  NextReadBuffer = ReadBuffer;
  NextWriteBuffer = WriteBuffer;
  memset( WriteBuffer, 0, 1024 * 1024 );

  printf( "\nTesting CancelVS1063Playback - vanilla...\n" );
  CancelVS1063Playback( NULL );

  // Were all ReadBuffer elements consumed as expected?
  printf( "Checking read values...\n");
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  if ( result ) {

    printf( "Read error - not all elements consumed!\n" );
    return result;
  }
  printf( "OK\n\n" );

  // Were all addresses and stuff written as expected?
  result |= CheckWrites( expectedWrites );

  return result;
}

BOOL testCancelPlaybackFLAC( VOID ) {

  BOOL result = FALSE;
  
  ULONG ReadBuffer[] = {
    VS1063_CODEC_SCI_HDAT1, 0x664C,
    VS1063_CODEC_ADDRESS_END_FILL, 0x1234,
    AMIGUS_CODEC_FIFO_USAGE, 17,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    AMIGUS_CODEC_FIFO_USAGE, 44,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    VS1063_CODEC_SCI_MODE, 0,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL, // 3 blocks of ignoring
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL, // 3 blocks of ignoring
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL, // 3 blocks of ignoring
    VS1063_CODEC_SCI_MODE, 0,
#if defined(__SASC)
    VS1063_CODEC_SCI_HDAT0, 0x4242,
    VS1063_CODEC_SCI_HDAT1, 0x4242,
#elif defined(__VBCC__)
    VS1063_CODEC_SCI_HDAT1, 0x4242,
    VS1063_CODEC_SCI_HDAT0, 0x4242,
#endif
    0xffFFffFF, 0xffFFffFF
  };
  ULONG expectedWrites[] = {
    1,          WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_ENABLE,
    4 * 768,    WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,          WriteCodecSPI_MARKER | VS1063_CODEC_SCI_MODE,  0x77770000 | VS1063_CODEC_F_SM_CANCEL,
    8,          WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434, // 3 blocks of ignoring
    8,          WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434, // 3 blocks of ignoring
    8,          WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434, // 3 blocks of ignoring
    8,          WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,          WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_DISABLE,
    0xffFFffFF, 0xffFFffFF
  };

  NextReadBuffer = ReadBuffer;
  NextWriteBuffer = WriteBuffer;
  memset( WriteBuffer, 0, 1024 * 1024 );

  printf( "\nTesting CancelVS1063Playback - FLAC + 3 ignores...\n" );
  CancelVS1063Playback( NULL );

  // Were all ReadBuffer elements consumed as expected?
  printf( "Checking read values...\n");
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  if ( result ) {

    printf( "Read error - not all elements consumed!\n" );
    return result;
  }
  printf( "OK\n\n" );

  // Were all addresses and stuff written as expected?
  result |= CheckWrites( expectedWrites );

  return result;
}

BOOL testCancelPlaybackReset( VOID ) {

  BOOL result = FALSE;
  
  ULONG ReadBuffer[] = {
    VS1063_CODEC_SCI_HDAT1, 0x4d50,
    VS1063_CODEC_ADDRESS_END_FILL, 0x1234,
    AMIGUS_CODEC_FIFO_USAGE, 17,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    VS1063_CODEC_SCI_MODE, 0,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
#if defined(__SASC)
    VS1063_CODEC_SCI_HDAT0, 0x4242,
    VS1063_CODEC_SCI_HDAT1, 0x4242,
#elif defined(__VBCC__)
    VS1063_CODEC_SCI_HDAT1, 0x4242,
    VS1063_CODEC_SCI_HDAT0, 0x4242,
#endif
    0xffFFffFF, 0xffFFffFF
  };
  ULONG expectedWrites[] = {
    1,      WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_ENABLE,
    768,    WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,      WriteCodecSPI_MARKER | VS1063_CODEC_SCI_MODE,  0x77770000 | VS1063_CODEC_F_SM_CANCEL,
    64*8,   WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,      WriteCodecSPI_MARKER | VS1063_CODEC_SCI_MODE,  0x77770000 | VS1063_CODEC_F_SM_RESET,
    1,      WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_DISABLE,
    0xffFFffFF, 0xffFFffFF
  };

  NextReadBuffer = ReadBuffer;
  NextWriteBuffer = WriteBuffer;
  memset( WriteBuffer, 0, 1024 * 1024 );

  printf( "\nTesting CancelVS1063Playback - vanilla...\n" );
  CancelVS1063Playback( NULL );

  // Were all ReadBuffer elements consumed as expected?
  printf( "Checking read values...\n");
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  if ( result ) {

    printf( "Read error - not all elements consumed!\n" );
    return result;
  }
  printf( "OK\n\n" );

  // Were all addresses and stuff written as expected?
  result |= CheckWrites( expectedWrites );

  return result;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {
 
  BOOL failed = FALSE;
 
  AmiGUS_MHI_Base = malloc( sizeof( struct AmiGUS_MHI ));
  memset( AmiGUS_MHI_Base, 0, sizeof( struct AmiGUS_MHI ));

  WriteBuffer = malloc( 1024 * 1024 );
 
  if ( !AmiGUS_MHI_Base ) {
 
    printf( "Memory allocation failed!" );
    return 20;
  }
  failed |= testCancelPlaybackVanilla();
  failed |= testCancelPlaybackFLAC();
  failed |= testCancelPlaybackReset();

  free( WriteBuffer );
  free( AmiGUS_MHI_Base );
 
  return ( failed ) ? 15 : 0;
}
