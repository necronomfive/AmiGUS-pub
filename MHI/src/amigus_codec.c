/*
 * This file is part of the mhiamigus.library.
 *
 * mhiamigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiamigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiamigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <proto/expansion.h>

#include "amigus_codec.h"
#include "amigus_hardware.h"
#include "amigus_mhi.h"
#include "amigus_vs1063.h"
#include "debug.h"
#include "errors.h"
#include "interrupt.h"
#include "support.h"

/******************************************************************************
 * Codec convenience functions - public function definitions.
 *****************************************************************************/

VOID StartAmiGusCodecPlayback( struct AmiGUS_MHI_Handle * handle ) {

  APTR card = handle->agch_CardBase;

  WriteReg16( card,
              AMIGUS_CODEC_FIFO_RESET,
              AMIGUS_CODEC_FIFO_F_RESET_STROBE );
  WriteReg16( card,
              AMIGUS_CODEC_FIFO_WATERMARK,
              AMIGUS_CODEC_PLAY_FIFO_WORDS >> 1 );
  WriteReg32( card,
              AMIGUS_CODEC_TIMER_RELOAD,
              0xffFFffFF );
  WriteReg16( card,
              AMIGUS_CODEC_TIMER_CONTROL,
              AMIGUS_TIMER_STOP | AMIGUS_TIMER_ONCE );
  WriteReg16( card,
              AMIGUS_CODEC_INT_ENABLE,
              AMIGUS_INT_F_CLEAR
              | AMIGUS_CODEC_INT_F_FIFO_EMPTY
              | AMIGUS_CODEC_INT_F_FIFO_FULL
              | AMIGUS_CODEC_INT_F_FIFO_WATERMRK
              | AMIGUS_CODEC_INT_F_SPI_FINISH
              | AMIGUS_CODEC_INT_F_VS1063_DRQ
              | AMIGUS_CODEC_INT_F_TIMER );
  WriteReg16( card,
              AMIGUS_CODEC_INT_CONTROL,
              AMIGUS_INT_F_CLEAR
              | AMIGUS_CODEC_INT_F_FIFO_EMPTY
              | AMIGUS_CODEC_INT_F_FIFO_FULL
              | AMIGUS_CODEC_INT_F_FIFO_WATERMRK
              | AMIGUS_CODEC_INT_F_SPI_FINISH
              | AMIGUS_CODEC_INT_F_VS1063_DRQ
              | AMIGUS_CODEC_INT_F_TIMER );
  WriteReg16( card,
              AMIGUS_CODEC_INT_ENABLE,
              AMIGUS_INT_F_SET
              | AMIGUS_CODEC_INT_F_FIFO_EMPTY
              | AMIGUS_CODEC_INT_F_FIFO_WATERMRK );
  FillCodecBuffer( handle );
  WriteReg16( card,
              AMIGUS_CODEC_FIFO_CONTROL,
              AMIGUS_CODEC_FIFO_F_DMA_ENABLE );
}

VOID StopAmiGusCodecPlayback( struct AmiGUS_MHI_Handle * handle ) {

  APTR card = handle->agch_CardBase;

  // Deactivate interrupts
  WriteReg16( card,
              AMIGUS_CODEC_INT_ENABLE,
              AMIGUS_INT_F_CLEAR
              | AMIGUS_CODEC_INT_F_FIFO_EMPTY
              | AMIGUS_CODEC_INT_F_FIFO_FULL
              | AMIGUS_CODEC_INT_F_FIFO_WATERMRK
              | AMIGUS_CODEC_INT_F_SPI_FINISH
              | AMIGUS_CODEC_INT_F_VS1063_DRQ
              | AMIGUS_CODEC_INT_F_TIMER );
  WriteReg16( card,
              AMIGUS_CODEC_INT_CONTROL,
              AMIGUS_INT_F_CLEAR
              | AMIGUS_CODEC_INT_F_FIFO_EMPTY
              | AMIGUS_CODEC_INT_F_FIFO_FULL
              | AMIGUS_CODEC_INT_F_FIFO_WATERMRK
              | AMIGUS_CODEC_INT_F_SPI_FINISH
              | AMIGUS_CODEC_INT_F_VS1063_DRQ
              | AMIGUS_CODEC_INT_F_TIMER );
  // Abort playback by codec
  CancelVS1063Playback( card );
  // Original AmiGUS "stop playback" functionality
  WriteReg16( card,
              AMIGUS_CODEC_FIFO_CONTROL,
              AMIGUS_CODEC_FIFO_F_DMA_DISABLE );
  // Flush card's buffers
  WriteReg16( card,
              AMIGUS_CODEC_FIFO_RESET,
              AMIGUS_CODEC_FIFO_F_RESET_STROBE );
}

VOID SleepCodecTicks( APTR amiGUS, ULONG ticks ) {

  LONG rounds = 0;

  LOG_V(( "V: Sleeping for %ld ticks\n", ticks ));
  WriteReg32( amiGUS, AMIGUS_CODEC_TIMER_RELOAD, ticks );
  WriteReg16( amiGUS,
              AMIGUS_CODEC_INT_CONTROL,
              AMIGUS_INT_F_CLEAR
              | AMIGUS_CODEC_INT_F_TIMER );
  WriteReg16( amiGUS, 
              AMIGUS_CODEC_TIMER_CONTROL,
              AMIGUS_TIMER_ONCE | AMIGUS_TIMER_START );
  while ( !( AMIGUS_CODEC_INT_F_TIMER 
          & ReadReg16( amiGUS, AMIGUS_CODEC_INT_CONTROL ))) {

    ++rounds;
  }
  LOG_V(( "V: Woke up after %ld rounds.\n", rounds ));
}
