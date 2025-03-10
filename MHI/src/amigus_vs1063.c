/*
 * This file is part of the mhiAmiGUS.library driver.
 *
 * mhiAmiGUS.library driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library driver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "amigus_hardware.h"
#include "amigus_vs1063.h"
#include "debug.h"
#include "support.h"

UWORD ReadVS1063Mem( APTR amiGUS, UWORD address ) {

  WriteCodecSPI( amiGUS, VS1063_CODEC_SCI_WRAMADDR, address );
  return ReadCodecSPI( amiGUS, VS1063_CODEC_SCI_WRAM );
}

VOID WriteVS1063Mem( APTR amiGUS, UWORD address, UWORD value ) {

  WriteCodecSPI( amiGUS, VS1063_CODEC_SCI_WRAMADDR, address );
  WriteCodecSPI( amiGUS, VS1063_CODEC_SCI_WRAM, value );
}

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

VOID UpdateVS1063Equalizer( APTR amiGUS, UWORD equalizerLevel, WORD value ) {

  LOG_V(( "V: Updating EQ5 level 0x%04lx = %ld\n", equalizerLevel, value ));
  WriteVS1063Mem( amiGUS, 
                  equalizerLevel,
                  value );
  WriteVS1063Mem( amiGUS,
                  VS1063_CODEC_ADDRESS_EQ5_UPDATE,
                  VS1063_CODEC_F_EQ5_UPD_STROBE );
}

ULONG GetVS1063EndFill( APTR amiGUS ) {

    ULONG endFill = ReadVS1063Mem( amiGUS, VS1063_CODEC_ADDRESS_END_FILL );
    endFill &= 0x000000FF;
    endFill |= ( endFill << 24 ) | ( endFill << 16 ) | ( endFill << 8 );
    return endFill;
  }
  
  /* Private function definitions: */
  
  VOID CancelVS1063Playback( APTR amiGUS ) {
  
    UWORD sciMode;
    ULONG i;
    ULONG j;
  
    // Need to know what is playing - page 50.
    // For FLAC, according to page 50 SCI_HDAT1 = 0x664C,
    // send 12288 end fill bytes according to page 57,
    // for all others, send >= 2052 end fill bytes.
    // Will use 3072 BYTEs = 768 LONGs, 
    // 'cause 3072 * 4 = 12288
    const UWORD format = ReadCodecSPI( amiGUS, VS1063_CODEC_SCI_HDAT1 );
    const ULONG blockCount = ( 0x664C == format) ? 4: 1;
    const ULONG blockSize = 768;
  
    // Now we need to tell the VS1063 to cleanly (!) stop
    // so...
    // Trigger end of 11.5.1 Playing a Whole File - page 57
  
    // step 2
    ULONG endFill = GetVS1063EndFill( amiGUS );
  
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
      sciMode = ReadVS1063Mem( amiGUS, VS1063_CODEC_SCI_MODE );
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
  
  VOID ResetVS1063( APTR amiGUS ) {
  
    LOG_D(( "D: Resetting VS1063 codec...\n"));
    WriteCodecSPI( amiGUS,
                   VS1063_CODEC_SCI_MODE,
                   VS1063_CODEC_F_SM_RESET );
    // page 56 - 11.3 Software Reset
    Sleep( 4 );
    /*
    // TODO: do we want / need that?
    LOG_D(( "D: ... patching ...\n"));
    ApplyCompressedVS1063Patch( amiGUS, VS1063Patch20191204 );
    */
    LOG_D(( "D: ... done.\n"));
  }
  