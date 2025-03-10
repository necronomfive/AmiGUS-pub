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

#include <proto/expansion.h>

#include "amigus_codec.h"
#include "amigus_hardware.h"
#include "amigus_mhi.h"
#include "amigus_vs1063.h"
#include "debug.h"
#include "errors.h"
#include "interrupt.h"
#include "support.h"

LONG FindAmiGusCodec( struct AmiGUSBase * amiGUSBase ) {

  struct ConfigDev *configDevice = 0;
  ULONG serial;
  UBYTE minute;
  UBYTE hour;
  UBYTE day;
  UBYTE month;
  UWORD year;

  configDevice = FindConfigDev( configDevice,
                                AMIGUS_MANUFACTURER_ID,
                                AMIGUS_CODEC_PRODUCT_ID );
  if ( !configDevice ) {

    LOG_E(("E: AmiGUS not found\n"));
    return EAmiGUSNotFound;
  }
  if (   ( AMIGUS_MANUFACTURER_ID != configDevice->cd_Rom.er_Manufacturer )
      || ( AMIGUS_CODEC_PRODUCT_ID != configDevice->cd_Rom.er_Product ) 
     ) {

    LOG_E(("E: AmiGUS detection failed\n"));
    return EAmiGUSDetectError;
  }

  serial = configDevice->cd_Rom.er_SerialNumber;
  if ( AMIGUS_MHI_FIRMWARE_MINIMUM > serial ) {

    LOG_E(( "E: AmiGUS firmware expected %08lx, actual %08lx\n",
            AMIGUS_MHI_FIRMWARE_MINIMUM, serial ));
    return EAmiGUSFirmwareOutdated;
  }

  LOG_V(("V: AmiGUS firmware %08lx\n", serial));
  minute = ( UBYTE )(( serial & 0x0000003Ful )       );
  hour   = ( UBYTE )(( serial & 0x000007C0ul ) >>  6 );
  day    = ( UBYTE )(( serial & 0x0000F800ul ) >> 11 );
  month  = ( UBYTE )(( serial & 0x000F0000ul ) >> 16 );
  year   = ( UWORD )(( serial & 0xFFF00000ul ) >> 20 );
  LOG_I(("I: AmiGUS firmware date %04ld-%02ld-%02ld, %02ld:%02ld\n",
         year, month, day, hour, minute));

  amiGUSBase->agb_CardBase = (struct AmiGUS *)configDevice->cd_BoardAddr;
  LOG_I(( "I: AmiGUS found at 0x%08lx\n",
          amiGUSBase->agb_CardBase ));
  LOG_V(( "V: AmiGUS address stored at 0x%08lx\n",
          &( amiGUSBase->agb_CardBase )));
  amiGUSBase->agb_UsageCounter = 0;

  return ENoError;
}

VOID StartAmiGusCodecPlayback( VOID ) {

  APTR amiGUS = AmiGUSBase->agb_CardBase;
  WriteReg16( amiGUS,
              AMIGUS_CODEC_FIFO_RESET,
              AMIGUS_CODEC_FIFO_F_RESET_STROBE );
  WriteReg16( amiGUS,
              AMIGUS_CODEC_FIFO_WATERMARK,
              AMIGUS_CODEC_PLAY_FIFO_WORDS >> 1 );
  WriteReg16( amiGUS,
              AMIGUS_CODEC_INT_ENABLE,
              AMIGUS_INT_F_CLEAR
              | AMIGUS_CODEC_INT_F_FIFO_EMPTY
              | AMIGUS_CODEC_INT_F_FIFO_FULL
              | AMIGUS_CODEC_INT_F_FIFO_WATERMRK
              | AMIGUS_CODEC_INT_F_SPI_FINISH
              | AMIGUS_CODEC_INT_F_VS1063_DRQ );
  WriteReg16( amiGUS,
              AMIGUS_CODEC_INT_CONTROL,
              AMIGUS_INT_F_CLEAR
              | AMIGUS_CODEC_INT_F_FIFO_EMPTY
              | AMIGUS_CODEC_INT_F_FIFO_FULL
              | AMIGUS_CODEC_INT_F_FIFO_WATERMRK
              | AMIGUS_CODEC_INT_F_SPI_FINISH
              | AMIGUS_CODEC_INT_F_VS1063_DRQ );
  WriteReg16( amiGUS,
              AMIGUS_CODEC_INT_ENABLE,
              AMIGUS_INT_F_SET
              | AMIGUS_CODEC_INT_F_FIFO_EMPTY
              | AMIGUS_CODEC_INT_F_FIFO_WATERMRK );
  HandlePlayback();
  WriteReg16( amiGUS,
              AMIGUS_CODEC_FIFO_CONTROL,
              AMIGUS_CODEC_FIFO_F_DMA_ENABLE );
}

VOID StopAmiGusCodecPlayback( VOID ) {

  APTR amiGUS = AmiGUSBase->agb_CardBase;

  // Original AmiGUS "stop playback" functionality
  WriteReg16( amiGUS,
              AMIGUS_CODEC_FIFO_CONTROL,
              AMIGUS_CODEC_FIFO_F_DMA_DISABLE );
  WriteReg16( amiGUS,
              AMIGUS_CODEC_INT_ENABLE,
              AMIGUS_INT_F_CLEAR
              | AMIGUS_CODEC_INT_F_FIFO_EMPTY
              | AMIGUS_CODEC_INT_F_FIFO_FULL
              | AMIGUS_CODEC_INT_F_FIFO_WATERMRK
              | AMIGUS_CODEC_INT_F_SPI_FINISH
              | AMIGUS_CODEC_INT_F_VS1063_DRQ );
  WriteReg16( amiGUS,
              AMIGUS_CODEC_INT_CONTROL,
              AMIGUS_INT_F_CLEAR
              | AMIGUS_CODEC_INT_F_FIFO_EMPTY
              | AMIGUS_CODEC_INT_F_FIFO_FULL
              | AMIGUS_CODEC_INT_F_FIFO_WATERMRK
              | AMIGUS_CODEC_INT_F_SPI_FINISH
              | AMIGUS_CODEC_INT_F_VS1063_DRQ );  
  WriteReg16( amiGUS,
              AMIGUS_CODEC_FIFO_RESET,
              AMIGUS_CODEC_FIFO_F_RESET_STROBE );
  CancelVS1063Playback( amiGUS );
}
