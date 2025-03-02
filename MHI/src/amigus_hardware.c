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
#include "debug.h"
#include "SDI_compiler.h"

UWORD ReadReg16( APTR amiGUS, ULONG offset ) {

  return *(( UWORD * )(( ULONG ) amiGUS + offset ));
}

VOID WriteReg16( APTR amiGUS, ULONG offset, UWORD value ) {

  *(( UWORD * )(( ULONG ) amiGUS + offset )) = value;
}

ULONG ReadReg32( APTR amiGUS, ULONG offset ) {

  return *(( ULONG * )(( ULONG ) amiGUS + offset ));
}

VOID WriteReg32( APTR amiGUS, ULONG offset, ULONG value ) {

  *(( ULONG * )(( ULONG ) amiGUS + offset )) = value;
}

INLINE VOID WriteSPI(
  APTR amiGUS,
  UWORD SPIregister,
  UWORD SPIvalue,
  UWORD blockedSPImask,
  UWORD offsetSPIstatus,
  UWORD offsetSPIaddress,
  UWORD offsetSPIwrite,
  UWORD offsetSPItrigger ) {

  UWORD status;

  do {

    status = ReadReg16( amiGUS, offsetSPIstatus );

  } while ( status & blockedSPImask );
  WriteReg16( amiGUS, offsetSPIaddress, SPIregister );
  WriteReg16( amiGUS, offsetSPIwrite, SPIvalue );
  WriteReg16( amiGUS, offsetSPItrigger, AMIGUS_CODEC_SPI_STROBE );
}

VOID WriteCodecSPI( APTR amiGUS, UWORD SPIregister, UWORD SPIvalue ) {

  WriteSPI(
    amiGUS,
    SPIregister,
    SPIvalue,
    AMIGUS_CODEC_SPI_F_DREQ | AMIGUS_CODEC_SPI_F_BUSY,
    AMIGUS_CODEC_SPI_STATUS,
    AMIGUS_CODEC_SPI_ADDRESS, 
    AMIGUS_CODEC_SPI_WRITE_DATA,
    AMIGUS_CODEC_SPI_WRITE_TRIGGER );
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

/*
 * ---------------------------------------------------
 * Data definitions
 * ---------------------------------------------------
 */

/*
 * Array of supported sample rates, shall always be in 
 * sync with 
 * - AMIGUS_SAMPLE_RATE_* in amigus_hardware.h and
 * - AMIGUS_AHI_NUM_SAMPLE_RATES in amigus_public.h.
 */
const LONG AmiGUSSampleRates[ AMIGUS_PCM_SAMPLE_RATE_COUNT ] = {

   8000, // AMIGUS_PCM_SAMPLE_RATE_8000  @ index 0x0000
  11025, // AMIGUS_PCM_SAMPLE_RATE_11025 @ index 0x0001
  16000, // AMIGUS_PCM_SAMPLE_RATE_16000 @ index 0x0002
  22050, // AMIGUS_PCM_SAMPLE_RATE_22050 @ index 0x0003
  24000, // AMIGUS_PCM_SAMPLE_RATE_24000 @ index 0x0004
  32000, // AMIGUS_PCM_SAMPLE_RATE_32000 @ index 0x0005
  44100, // AMIGUS_PCM_SAMPLE_RATE_44100 @ index 0x0006
  48000, // AMIGUS_PCM_SAMPLE_RATE_48000 @ index 0x0007
  96000  // AMIGUS_PCM_SAMPLE_RATE_96000 @ index 0x0008
};

const STRPTR AmiGUSOutputs[ AMIGUS_OUTPUTS_COUNT ] = {

  "Line Out"
};

const STRPTR AmiGUSInputs[ AMIGUS_INPUTS_COUNT ] = {

  "External / see Mixer",
  "MHI / Codec",
  "WaveTable",
  // "AHI / PCM",                    // Requires splitting 1 card for 2 clients
  "ALL / What-You-Hear"
};

const UWORD AmiGUSInputFlags[ AMIGUS_INPUTS_COUNT ] = {

  AMIGUS_PCM_S_REC_F_ADC_SRC,        // External above
  AMIGUS_PCM_S_REC_F_CODEC_SRC,
  AMIGUS_PCM_S_REC_F_WAVETABLE_SRC,
  // AMIGUS_PCM_S_REC_F_AHI_SRC,
  AMIGUS_PCM_S_REC_F_MIXER_SRC       // What-You-Hear
};
