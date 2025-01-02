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

#include <proto/expansion.h>

#include "amigus_pcm.h"
#include "debug.h"
#include "errors.h"

LONG FindAmiGusPcm(struct AmiGUSBasePrivate *amiGUSBase) {

  struct ConfigDev *configDevice = 0;
  ULONG serial = 0;
  UBYTE minute = 0;
  UBYTE hour   = 0;
  UBYTE day    = 0;
  UBYTE month  = 0;
  UWORD year   = 0;

  configDevice = FindConfigDev( configDevice,
                                AMIGUS_MANUFACTURER_ID,
                                AMIGUS_MAIN_PRODUCT_ID );
  if ( !configDevice ) {

    LOG_E(("E: AmiGUS not found\n"));
    return EAmiGUSNotFound;
  }
  if (   ( AMIGUS_MANUFACTURER_ID != configDevice->cd_Rom.er_Manufacturer )
      || ( AMIGUS_MAIN_PRODUCT_ID != configDevice->cd_Rom.er_Product ) 
     ) {

    LOG_E(("E: AmiGUS detection failed\n"));
    return EAmiGUSDetectError;
  }

  serial = configDevice->cd_Rom.er_SerialNumber;
  if ( AMIGUS_AHI_FIRMWARE_MINIMUM > serial ) {

    LOG_E(("E: AmiGUS firmware expected %08lx, actual %08lx\n",
           AMIGUS_AHI_FIRMWARE_MINIMUM, serial));
    return EAmiGUSFirmwareOutdated;
  }

  LOG_V(("V: AmiGUS firmware %08lx\n", serial));
  minute = (UBYTE)((serial & 0x0000003Ful)      );
  hour   = (UBYTE)((serial & 0x000007C0ul) >>  6);
  day    = (UBYTE)((serial & 0x0000F800ul) >> 11);
  month  = (UBYTE)((serial & 0x000F0000ul) >> 16);
  year   = (UWORD)((serial & 0xFFF00000ul) >> 20);
  LOG_I(("I: AmiGUS firmware date %04ld-%02ld-%02ld, %02ld:%02ld\n",
         year, month, day, hour, minute));

  amiGUSBase->agb_CardBase = (struct AmiGUS *)configDevice->cd_BoardAddr;
  LOG_I(( "I: AmiGUS found at 0x%08lx\n",
          amiGUSBase->agb_CardBase ));
  LOG_V(( "V: AmiGUS address stored at 0x%08lx\n",
          &(amiGUSBase->agb_CardBase )));
  amiGUSBase->agb_UsageCounter = 0;

  return ENoError;
}

VOID StartAmiGusPcmPlayback( VOID ) {

  ULONG i;
  ULONG prefillSize = 12; /* in LONGs */ 
  APTR amiGUS = AmiGUSBase->agb_CardBase;
  LOG_D(("D: Init AmiGUS @ 0x%08lx\n", amiGUS));

  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_SAMPLE_RATE,
              AMIGUS_PCM_SAMPLE_F_DISABLE );
  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_FIFO_RESET,
              AMIGUS_PCM_PLAY_FIFO_RESET );
  // Idea: Watermark is here 6 words aka 3 longs,
  //       2 24bit samples, ... 
  // Cool, always a fit for all sample widths.
  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_FIFO_WATERMARK,
              // Watermark is WORDs, so using LONG value means half
              prefillSize );
  WriteReg16( amiGUS, 
              AMIGUS_PCM_MAIN_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_F_MASK_CLEAR
            | AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_FULL
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_MAIN_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_F_MASK_CLEAR
            | AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_FULL
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_MAIN_INT_ENABLE,
              /* Now set some! */
              AMIGUS_INT_F_MASK_SET
            | AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK );

  // Now write twice the amount of data into FIFO to kick off playback
  for ( i = prefillSize; 0 < i; --i ) {

    WriteReg32( amiGUS,
                AMIGUS_PCM_PLAY_FIFO_WRITE,
                0L );
  }
  // Use correct sample settings, prefill is selected to match all
  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_SAMPLE_FORMAT,
              AmiGUSBase->agb_HwSampleFormat );

  // Start playback finally
  AmiGUSBase->agb_StateFlags &= AMIGUS_AHI_F_PLAY_STOP_MASK;
  AmiGUSBase->agb_StateFlags |= AMIGUS_AHI_F_PLAY_STARTED;
  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_SAMPLE_RATE,
              AmiGUSBase->agb_HwSampleRateId
            | AMIGUS_PCM_S_PLAY_F_INTERPOLATE
            | AMIGUS_PCM_SAMPLE_F_ENABLE );
}

VOID StopAmiGusPcmPlayback( VOID ) {

  APTR amiGUS = AmiGUSBase->agb_CardBase;
  LOG_D(("D: Stop AmiGUS @ 0x%08lx\n", amiGUS));

  WriteReg16( amiGUS, 
              AMIGUS_PCM_PLAY_SAMPLE_RATE, 
              AMIGUS_PCM_SAMPLE_F_DISABLE );
  WriteReg16( amiGUS, 
              AMIGUS_PCM_MAIN_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_FULL
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_MAIN_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_FULL
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_FIFO_RESET,
              0x0000 );
  AmiGUSBase->agb_StateFlags &= AMIGUS_AHI_F_PLAY_STOP_MASK;
}
