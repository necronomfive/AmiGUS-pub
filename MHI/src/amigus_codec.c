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
  ULONG endFill;
  UWORD sciMode;
  ULONG i;
  ULONG j;
  UWORD format;
  ULONG repeats;

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
  WriteReg16( amiGUS,
              AMIGUS_CODEC_FIFO_CONTROL,
              AMIGUS_CODEC_FIFO_F_DMA_ENABLE );

  // Trigger 11.5.1 Playing a Whole File - page 57
  format = ReadCodecSPI( amiGUS, VS1063_CODEC_SCI_HDAT1 );
  if ( 0x664C == format) { // FLAC according to page 50
    // For FLAC, send 12288 end fill bytes - page 57:
    repeats = 4;

  } else {
    // All others: send >= 2052 end fill bytes - page 57:
    // Will use 3072 BYTEs = 768 LONGs, 
    // 'cause 3072 * 4 = 12288
    repeats = 1;
  }
  // step 2
  endFill = ReadVS1063Mem( amiGUS, VS1063_CODEC_ADDRESS_END_FILL );
  endFill &= 0x000000FF;
  endFill |= ( endFill << 24 ) | ( endFill << 16 ) | ( endFill << 8 );
  // step 3
  for ( j = 0; j < repeats; j++ ) {
    LOG_V(( "V: End of file step 3.%ld\n", j ));
    for ( i = 0; i < 768; ++i) {

      WriteReg32( amiGUS, AMIGUS_CODEC_FIFO_WRITE, endFill );
    }
    while( ReadReg32( amiGUS, AMIGUS_CODEC_FIFO_USAGE ) );
  }
  // step 4
  LOG_V(( "V: End of file step 4\n" ));
  sciMode = ReadCodecSPI( amiGUS, VS1063_CODEC_SCI_MODE );
  sciMode |= VS1063_CODEC_F_SM_CANCEL;
  WriteCodecSPI( amiGUS, VS1063_CODEC_SCI_MODE, sciMode );

  for ( j = 0; j < 64; ++j ) {
    // step 5
    LOG_V(( "V: End of file step 5.%ld\n", j ));
    for ( i = 0; i < 8; ++i) {

      WriteReg32( amiGUS, AMIGUS_CODEC_FIFO_WRITE, endFill );
    }
    // step 6
    sciMode = ReadVS1063Mem( amiGUS, VS1063_CODEC_SCI_MODE );
    if ( !( sciMode & VS1063_CODEC_F_SM_CANCEL )) {

      LOG_V(( "V: End of file step 6\n" ));
      break;
    }
  }
  if ( sciMode & VS1063_CODEC_F_SM_CANCEL ) {

    LOG_V(( "V: End of file failed - reset\n" ));
    WriteCodecSPI( amiGUS,
                   VS1063_CODEC_SCI_MODE,
                   VS1063_CODEC_F_SM_RESET );
    // page 56 - 11.3 Software Reset
    Sleep( 4 );
    // TODO: Apply patches here!!!
  }
  WriteReg16( amiGUS,
              AMIGUS_CODEC_FIFO_CONTROL,
              AMIGUS_CODEC_FIFO_F_DMA_DISABLE );
  LOG_V(( "V: Playback ended, HDAT0 = 0x%04lx, HDAT1 = 0x%04lx\n",
          ReadCodecSPI( amiGUS, VS1063_CODEC_SCI_HDAT0 ),
          ReadCodecSPI( amiGUS, VS1063_CODEC_SCI_HDAT1 )));
}
