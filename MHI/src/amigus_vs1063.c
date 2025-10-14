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

#include "amigus_codec.h"
#include "amigus_hardware.h"
#include "amigus_vs1063.h"
#include "debug.h"
#include "support.h"

/******************************************************************************
 * VS1063 codec convenience functions - private functions.
 *****************************************************************************/

/**
 * Resets the codec.
 * Usually not required, only if cancelling playback during a song fails...
 *
 * @param amiGUS         Pointer to the AmiGUS codec's register bank.
 */
VOID ResetVS1063( APTR amiGUS ) {

  ULONG circle = 0;

  LOG_D(( "D: Resetting VS1063 codec from mode 0x%04lx...\n",
          ReadCodecSPI( amiGUS, VS1063_CODEC_SCI_MODE )));
  WriteReg16( amiGUS,
              AMIGUS_CODEC_INT_CONTROL,
              AMIGUS_CODEC_INT_F_VS1063_DRQ |
              AMIGUS_INT_F_CLEAR );
  WriteCodecSPI( amiGUS,
                 VS1063_CODEC_SCI_MODE,
                 VS1063_CODEC_F_SM_CLK_RANGE |
                 VS1063_CODEC_F_SM_SDINEW |
                 VS1063_CODEC_F_SM_RESET );
  // page 56 - 11.3 Software Reset
  SleepCodecTicks( amiGUS, VS1063_CODEC_RESET_DELAY_TICKS );
  // and wait for DREQ
  for ( ; ; ) {
    UWORD status = ReadReg16( amiGUS, AMIGUS_CODEC_INT_CONTROL );
    ++circle;
    if ( status & AMIGUS_CODEC_INT_F_VS1063_DRQ ) {

      LOG_D(( "D: ... done, %ld circles.\n", circle ));
      return;
    }
    if ( circle & 0x00000400 ) {

      LOG_W(( "W: Status flag is 0x%04lx after %ld circles.\n", status, circle ));
    }
    if ( circle > 1000000 ) {

      LOG_E(( "E: Giving up - this is bad!\n" ));
      return;
    }
  }
}

/******************************************************************************
 * VS1063 codec convenience functions - public function definitions.
 *****************************************************************************/

VOID InitVS1063Codec( APTR amiGUS ) {

  // Set SC_MULT to XTALI x 5.0 in SC_CLOCKF,
  // see VS1063a Datasheet, Version: 1.32, 2024-01-31, page 48
  WriteCodecSPI( amiGUS,
                 VS1063_CODEC_SCI_CLOCKF,
                 VS1063_CODEC_F_SC_MULT_5_0X );
  // Enable I2S Interface at 192kHz sample rate,
  // see VS1063a Datasheet, Version: 1.32, 2024-01-31, page 86
  WriteVS1063Mem( amiGUS,
                  VS1063_CODEC_ADDRESS_GPIO_DDR,
                  VS1063_CODEC_F_GPIO_DDR_192k );
  WriteVS1063Mem( amiGUS,
                  VS1063_CODEC_ADDRESS_I2S_CONFIG,
                  VS1063_CODEC_F_I2S_CONFIG_RESET );
  WriteVS1063Mem( amiGUS,
                  VS1063_CODEC_ADDRESS_I2S_CONFIG,
                  VS1063_CODEC_F_I2S_CONFIG_192k );
}

VOID InitVS1063Equalizer( APTR amiGUS, BOOL enable, const WORD * settings ) {

  UWORD oldPlayMode = ReadVS1063Mem( amiGUS,
                                     VS1063_CODEC_ADDRESS_PLAY_MODE );
  UWORD newPlayMode = ( enable )
                      ? ( oldPlayMode | VS1063_CODEC_F_PL_MO_EQ5_ENABLE )
                      : ( oldPlayMode & ~VS1063_CODEC_F_PL_MO_EQ5_ENABLE );
  UWORD wasEnabled = oldPlayMode & VS1063_CODEC_F_PL_MO_EQ5_ENABLE;

  LOG_V(( "V: Old PlayMode 0x%04lx = 0x%04lx\n",
          VS1063_CODEC_ADDRESS_PLAY_MODE, oldPlayMode ));
  if ( enable ) {

    UWORD i;
    for ( i = 0 ; i < 9 ; ++i ) {

      LOG_V(( "V: Setting 0x%04lx = %ld\n",
              VS1063_CODEC_ADDRESS_EQ5_LEVEL1 + i, settings[ i ] ));
      WriteVS1063Mem( amiGUS, 
                      VS1063_CODEC_ADDRESS_EQ5_LEVEL1 + i,
                      settings[ i ] );
    }
  }

  if (( wasEnabled ) && ( enable )) {

    LOG_V(( "V: Updating EQ5 only \n" ));
    WriteVS1063Mem( amiGUS,
                    VS1063_CODEC_ADDRESS_EQ5_UPDATE,
                    VS1063_CODEC_F_EQ5_UPD_STROBE );

  } else {

    LOG_V(( "V: New PlayMode 0x%04lx = 0x%04lx\n",
            VS1063_CODEC_ADDRESS_PLAY_MODE, newPlayMode ));
    WriteVS1063Mem( amiGUS,
                    VS1063_CODEC_ADDRESS_PLAY_MODE,
                    newPlayMode );
  }
}

VOID SetVS1063Equalizer( APTR amiGUS, UWORD equalizerLevel, WORD value ) {

  LOG_V(( "V: Setting EQ5 level 0x%04lx = %ld\n", equalizerLevel, value ));
  WriteVS1063Mem( amiGUS, 
                  equalizerLevel,
                  value );
  WriteVS1063Mem( amiGUS,
                  VS1063_CODEC_ADDRESS_EQ5_UPDATE,
                  VS1063_CODEC_F_EQ5_UPD_STROBE );
}

VOID UpdateVS1063Equalizer( APTR amiGUS, UBYTE values[ 11 ] ) {

  UBYTE min = 255;
  UBYTE max = 0;
  WORD gain;
  UBYTE i;
  UWORD levels[ 5 ] = {

    VS1063_CODEC_ADDRESS_EQ5_LEVEL1,
    VS1063_CODEC_ADDRESS_EQ5_LEVEL2,
    VS1063_CODEC_ADDRESS_EQ5_LEVEL3,
    VS1063_CODEC_ADDRESS_EQ5_LEVEL4,
    VS1063_CODEC_ADDRESS_EQ5_LEVEL5
  };

  // Step 1: calculate min + max values for all bands
  for ( i = 0; i < 10; ++i ) {

    const UBYTE current = values[ i ];
    if ( current < min ) {

      min = current;
    }
    if ( current > max ) {

      max = current;
    }
  }

  // Step 2: calculate gain - remember: VERY limited in numerical range!!!
  gain = values[ 10 ];
  LOG_V(( "V: gain %ld, min %ld, max %ld\n", gain, min, max ));
  gain -= 50;
  gain *= 32;
  gain /= 50;
  if ( gain >= 0 ) {

    gain *= ( WORD )( 100 - max );

  } else {

    gain *= ( WORD ) min;
  }
  gain /= 50;
  LOG_V(( "V: resulting gain %ld\n", gain ));

  // Step 3: calculate and set final values!
  for ( i = 0; i < 10; i += 2 ) {

    // Originally:
    // -32 .. +32 = ((( 0 .. 100 ) * 2655 ) / 4096 ) - 32
    // gain = percent * 2655 / 4096 - 32
    // But here, we are combining 2 sliders,
    // so range is 0-200 and >> 13 to take the average :)
    WORD combined = values[ i ] + values[ i + 1 ];
    LONG intermediate = (( combined * 2655 ) >> 13 ) - 32;
    WORD final = ( WORD ) intermediate + gain;
    LOG_D(( "D: UpdateEQ: Calculated gain %ld = %ld for %ld%%\n",
            intermediate, final, combined ));

    SetVS1063Equalizer( amiGUS, levels[ i>>1 ], final );
  }
}

VOID UpdateVS1063VolumePanning( APTR amiGUS, UBYTE volume, UBYTE panning ) {

  UBYTE multiplierLeft = ( 100 - panning ) << 1;
  UBYTE multiplierRight = (( panning - 50 ) << 1 ) + 100;
  UBYTE left;
  UBYTE right;
  UWORD value;

  LOG_D(( "D: UpdateVolume: To %lu%% vol, %lu%% pan\n", volume, panning ));

  multiplierLeft = ( multiplierLeft > 100 ) ? 100 : multiplierLeft;
  multiplierRight = ( multiplierRight > 100 ) ? 100 : multiplierRight;

  left = ( volume * multiplierLeft ) / 100;
  left = AmiGUSVolumeMapping[ left ];
  right = ( volume * multiplierRight ) / 100;
  right = AmiGUSVolumeMapping[ right ];
  value = ( left << 8 ) | ( right & 0x00FF );

  LOG_V(( "V: left x %ld / 100 = 0x%02lx, "
          "right x %ld / 100 = 0x%02lx -> 0x%04lx\n",
          multiplierLeft, left, 
          multiplierRight, right, value ));
  WriteCodecSPI( amiGUS, VS1063_CODEC_SCI_VOL, value );
}

ULONG GetVS1063EndFill( APTR amiGUS ) {

  ULONG endFill = ReadVS1063Mem( amiGUS, VS1063_CODEC_ADDRESS_END_FILL );
  endFill &= 0x000000FF;
  endFill |= ( endFill << 24 ) | ( endFill << 16 ) | ( endFill << 8 );
  return endFill;
}

VOID CancelVS1063Playback( APTR amiGUS ) {

  UWORD sciMode;
  ULONG i;
  ULONG j;
  UWORD format;
  ULONG blockCount;
  const ULONG blockSize = 768;
  ULONG endFill;

  // Need to know what is playing - page 50.
  // For FLAC, according to page 50 SCI_HDAT1 = 0x664C,
  // send 12288 end fill bytes according to page 57,
  // for all others, send >= 2052 end fill bytes.
  // Will use 3072 BYTEs = 768 LONGs (hence blockSize), 
  // 'cause 3072 * 4 = 12288
  LOG_V(( "V: End of file step 1.\n" ));
  format = ReadCodecSPI( amiGUS, VS1063_CODEC_SCI_HDAT1 );
  blockCount = ( 0x664C == format) ? 4: 1;
  LOG_V(( "V: End of file step 1.%08lx.\n", format ));

  // Now we need to tell the VS1063 to cleanly (!) stop
  // so...
  // Trigger end of 11.5.1 Playing a Whole File - page 57

  // step 2
  LOG_V(( "V: End of file step 2.\n" ));
  endFill = GetVS1063EndFill( amiGUS );
  LOG_V(( "V: End of file step 2.%08lx.\n", endFill ));

  // Re-Enable DMA so we can feed end fill bytes
  WriteReg16( amiGUS,
              AMIGUS_CODEC_FIFO_CONTROL,
              AMIGUS_CODEC_FIFO_F_DMA_ENABLE );
  // step 3
  for ( j = 0; j < blockCount; j++ ) {
    LOG_V(( "V: End of file step 3.%ld\n", j ));
    for ( i = 0; i < blockSize; ++i) {

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
    sciMode = ReadCodecSPI( amiGUS, VS1063_CODEC_SCI_MODE );
    if ( !( sciMode & VS1063_CODEC_F_SM_CANCEL )) {

      LOG_V(( "V: End of file step 6\n" ));
      break;
    }
  }
  if ( sciMode & VS1063_CODEC_F_SM_CANCEL ) {

    LOG_V(( "V: End of file failed - reset\n" ));
    ResetVS1063( amiGUS );
  }
  WriteReg16( amiGUS,
              AMIGUS_CODEC_FIFO_CONTROL,
              AMIGUS_CODEC_FIFO_F_DMA_DISABLE );
  LOG_V(( "V: Playback ended, HDAT0 = 0x%04lx, HDAT1 = 0x%04lx\n",
          ReadCodecSPI( amiGUS, VS1063_CODEC_SCI_HDAT0 ),
          ReadCodecSPI( amiGUS, VS1063_CODEC_SCI_HDAT1 )));
}
