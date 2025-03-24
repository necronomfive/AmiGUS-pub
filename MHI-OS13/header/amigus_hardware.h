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

#ifndef AMIGUS_HARDWARE_H
#define AMIGUS_HARDWARE_H

#include <exec/types.h>

/*
 * defines are limited to 32 chars due to a SAS/C insufficiency !!!
 */
#define SASC_MAXIMUM_DEFINE_LENGTH_IS_32 12345678

/* AmiGUS Zorro IDs */

#define AMIGUS_MANUFACTURER_ID           2782

#define AMIGUS_MAIN_PRODUCT_ID           16
#define AMIGUS_HAGEN_PRODUCT_ID          17
#define AMIGUS_CODEC_PRODUCT_ID          18

/******************************************************************************
 * AmiGUS PCM hardware definitions below
 *****************************************************************************/

/* AmiGUS PCM Registers */
#define AMIGUS_PCM_INT_CONTROL           0x00
#define AMIGUS_PCM_INT_ENABLE            0x02

#define AMIGUS_PCM_PLAY_SAMPLE_FORMAT    0x04
#define AMIGUS_PCM_PLAY_SAMPLE_RATE      0x06
#define AMIGUS_PCM_PLAY_FIFO_RESET       0x08
#define AMIGUS_PCM_PLAY_FIFO_WATERMARK   0x0a
#define AMIGUS_PCM_PLAY_FIFO_WRITE       0x0c
#define AMIGUS_PCM_PLAY_FIFO_USAGE       0x10
#define AMIGUS_PCM_PLAY_VOLUME           0x12
#define AMIGUS_PCM_PLAY_VOLUME_LEFT      0x12
#define AMIGUS_PCM_PLAY_VOLUME_RIGHT     0x14

#define AMIGUS_PCM_REC_SAMPLE_FORMAT     0x80
#define AMIGUS_PCM_REC_SAMPLE_RATE       0x82
#define AMIGUS_PCM_REC_FIFO_RESET        0x88
#define AMIGUS_PCM_REC_FIFO_WATERMARK    0x8a
#define AMIGUS_PCM_REC_FIFO_READ         0x8c
#define AMIGUS_PCM_REC_FIFO_USAGE        0x90
#define AMIGUS_PCM_REC_VOLUME            0x84
#define AMIGUS_PCM_REC_VOLUME_LEFT       0x84
#define AMIGUS_PCM_REC_VOLUME_RIGHT      0x86

/* AmiGUS PCM Interrupt Flags */
#define AMIGUS_PCM_INT_F_PLAY_FIFO_EMPTY 0x0001
#define AMIGUS_PCM_INT_F_PLAY_FIFO_FULL  0x0002
#define AMIGUS_PCM_INT_F_PLAY_FIFO_WTRMK 0x0004
#define AMIGUS_PCM_INT_F_SPI_FINISH      0x0008
#define AMIGUS_PCM_INT_F_REC_FIFO_EMPTY  0x0010
#define AMIGUS_PCM_INT_F_REC_FIFO_FULL   0x0020
#define AMIGUS_PCM_INT_F_REC_FIFO_WTRMRK 0x0040
#define AMIGUS_PCM_INT_F_TIMER_ENABLE    0x4000
#define AMIGUS_INT_F_SET                 0x8000
#define AMIGUS_INT_F_CLEAR               0x0000

/* AmiGUS PCM Playback Sample Formats */
#define AMIGUS_PCM_S_PLAY_MONO_8BIT      0x0000
#define AMIGUS_PCM_S_PLAY_STEREO_8BIT    0x0001
#define AMIGUS_PCM_S_PLAY_MONO_16BIT     0x0002
#define AMIGUS_PCM_S_PLAY_STEREO_16BIT   0x0003
#define AMIGUS_PCM_S_PLAY_MONO_24BIT     0x0004
#define AMIGUS_PCM_S_PLAY_STEREO_24BIT   0x0005
/* AMIGUS PCM Playback Sample Format Flags */
#define AMIGUS_PCM_S_PLAY_F_SWAP_CH      0x0008
#define AMIGUS_PCM_S_PLAY_F_LITTLE_END   0x0010
#define AMIGUS_PCM_S_PLAY_FLAG_UNSIGNED  0x0020

/* AmiGUS PCM Recording Sample Formats */
#define AMIGUS_PCM_S_REC_MONO_8BIT       0x0006
#define AMIGUS_PCM_S_REC_STEREO_8BIT     0x0000
#define AMIGUS_PCM_S_REC_MONO_16BIT      0x0007
#define AMIGUS_PCM_S_REC_STEREO_16BIT    0x0001
#define AMIGUS_PCM_S_REC_MONO_24BIT      0x00FF
#define AMIGUS_PCM_S_REC_STEREO_24BIT    0x00FF

/* AmiGUS PCM Sample Rates */
#define AMIGUS_PCM_SAMPLE_RATE_8000      0x0000
#define AMIGUS_PCM_SAMPLE_RATE_11025     0x0001
#define AMIGUS_PCM_SAMPLE_RATE_16000     0x0002
#define AMIGUS_PCM_SAMPLE_RATE_22050     0x0003
#define AMIGUS_PCM_SAMPLE_RATE_24000     0x0004
#define AMIGUS_PCM_SAMPLE_RATE_32000     0x0005
#define AMIGUS_PCM_SAMPLE_RATE_44100     0x0006
#define AMIGUS_PCM_SAMPLE_RATE_48000     0x0007
#define AMIGUS_PCM_SAMPLE_RATE_96000     0x0008
#define AMIGUS_PCM_SAMPLE_RATE_COUNT     9
/* AmiGUS PCM Sample Rate Flags */
#define AMIGUS_PCM_SAMPLE_F_ENABLE       0x8000
#define AMIGUS_PCM_SAMPLE_F_DISABLE      0x0000

/* AmiGUS PCM Playback Sample Rate Flags */
#define AMIGUS_PCM_S_PLAY_F_INTERPOLATE  0x4000

/* AmiGUS PCM Recording Sample Rate Flags */
#define AMIGUS_PCM_S_REC_F_MIXER_SRC     0x0000
#define AMIGUS_PCM_S_REC_F_ADC_SRC       0x0010
#define AMIGUS_PCM_S_REC_F_CODEC_SRC     0x0020
#define AMIGUS_PCM_S_REC_F_WAVETABLE_SRC 0x0030
#define AMIGUS_PCM_S_REC_F_AHI_SRC       0x0040

/* FIFO Reset */
#define AMIGUS_PCM_FIFO_RESET_STROBE     0x0000

/* FIFO size */
#define AMIGUS_PCM_PLAY_FIFO_BYTES       8192
#define AMIGUS_PCM_PLAY_FIFO_WORDS       4096
#define AMIGUS_PCM_PLAY_FIFO_LONGS       2048

#define AMIGUS_PCM_REC_FIFO_BYTES        4096
#define AMIGUS_PCM_REC_FIFO_WORDS        2048
#define AMIGUS_PCM_REC_FIFO_LONGS        1024

#define AMIGUS_OUTPUTS_COUNT             1
#define AMIGUS_INPUTS_COUNT              4

/******************************************************************************
 * AmiGUS Codec hardware definitions below
 *****************************************************************************/

#define	AMIGUS_CODEC_INT_CONTROL         0x00
#define	AMIGUS_CODEC_INT_ENABLE          0x02

#define	AMIGUS_CODEC_FIFO_CONTROL        0x04
#define	AMIGUS_CODEC_FIFO_RESET          0x06
#define	AMIGUS_CODEC_FIFO_WATERMARK      0x08
#define	AMIGUS_CODEC_FIFO_WRITE          0x0a
#define	AMIGUS_CODEC_FIFO_USAGE          0x0e

#define AMIGUS_CODEC_SPI_ADDRESS         0x20
#define AMIGUS_CODEC_SPI_WRITE_DATA      0x22
#define AMIGUS_CODEC_SPI_WRITE_TRIGGER   0x24
#define AMIGUS_CODEC_SPI_READ_TRIGGER    0x26
#define AMIGUS_CODEC_SPI_READ_DATA       0x28
#define AMIGUS_CODEC_SPI_STATUS	         0x2a

#define AMIGUS_CODEC_SPI_STROBE          0x0000

/* AmiGUS Codec Interrupt Flags */
#define AMIGUS_CODEC_INT_F_FIFO_EMPTY    0x0001
#define AMIGUS_CODEC_INT_F_FIFO_FULL     0x0002
#define AMIGUS_CODEC_INT_F_FIFO_WATERMRK 0x0004
#define AMIGUS_CODEC_INT_F_SPI_FINISH    0x0008
#define AMIGUS_CODEC_INT_F_VS1063_DRQ    0x0010
// As above:
//      AMIGUS_INT_F_SPI_TRANSFER_FINISH 0x0008
//      AMIGUS_INT_F_SET                 0x8000
//      AMIGUS_INT_F_CLEAR               0x0000

/* AmiGUS Codec FIFO Steering Flags */
/* FIFO DMA */
#define AMIGUS_CODEC_FIFO_F_DMA_ENABLE   0x8000
#define AMIGUS_CODEC_FIFO_F_DMA_DISABLE  0x0000

/* FIFO Reset */
#define AMIGUS_CODEC_FIFO_F_RESET_STROBE 0x0000

/* AmiGUS Codec SPI Steering Flags */
#define AMIGUS_CODEC_SPI_F_DREQ          0x4000
#define AMIGUS_CODEC_SPI_F_BUSY          0x8000

/* FIFO size */
#define AMIGUS_CODEC_PLAY_FIFO_BYTES     4096
#define AMIGUS_CODEC_PLAY_FIFO_WORDS     2048
#define AMIGUS_CODEC_PLAY_FIFO_LONGS     1024

/**
 * VS1063 codec specifics
 * - all according to VS1063a Datasheet
 * - vs1063ds.pdf, Version: 1.32, 2024-01-31
 * - page references point into that
 */

// VS1063 codec's SDI - Serial Data Interface is SPI port for data

// VS1063 codec's SCIs - Serial Control Interface SPI ports, overview page 43
#define VS1063_CODEC_SCI_MODE           0x0000 // page 44
#define VS1063_CODEC_SCI_CLOCKF         0x0003 // page 48
#define VS1063_CODEC_SCI_WRAM           0x0006 // page 49
#define VS1063_CODEC_SCI_WRAMADDR       0x0007 // page 49
#define VS1063_CODEC_SCI_HDAT0          0x0008 // page 50
#define VS1063_CODEC_SCI_HDAT1          0x0009 // page 50

// VS1063 codec's addresses of memory mapped registers
// all parameter memory 0x1E00-0x1E3F is mapped to 0xC0C0-0xC0FF - page 49
#define VS1063_CODEC_ADDRESS_END_FILL   0xC0C6 // 0x1E06 p.70 + 57
#define VS1063_CODEC_ADDRESS_PLAY_MODE  0xC0C9 // 0x1E09 p.77
#define VS1063_CODEC_ADDRESS_EQ5_LEVEL1 0xC0D3 // 0x1E13 p.77 (-32 - +32)*0.5dB
#define VS1063_CODEC_ADDRESS_EQ5_FREQ1  0xC0D4 // 0x1E14 p.77      20 -   150Hz
#define VS1063_CODEC_ADDRESS_EQ5_LEVEL2 0xC0D5 // 0x1E15 p.77 (-32 - +32)*0.5dB
#define VS1063_CODEC_ADDRESS_EQ5_FREQ2  0xC0D6 // 0x1E16 p.77 -    50 -  1000Hz
#define VS1063_CODEC_ADDRESS_EQ5_LEVEL3 0xC0D7 // 0x1E17 p.77 (-32 - +32)*0.5dB
#define VS1063_CODEC_ADDRESS_EQ5_FREQ3  0xC0D8 // 0x1E18 p.77 -  1000 - 15000Hz
#define VS1063_CODEC_ADDRESS_EQ5_LEVEL4 0xC0D9 // 0x1E19 p.77 (-32 - +32)*0.5dB
#define VS1063_CODEC_ADDRESS_EQ5_FREQ4  0xC0DA // 0x1E1A p.77 -  2000 - 15000Hz
#define VS1063_CODEC_ADDRESS_EQ5_LEVEL5 0xC0DB // 0x1E1B p.77 (-32 - +32)*0.5dB
#define VS1063_CODEC_ADDRESS_EQ5_UPDATE 0xC0DC // 0x1E1C p.77 strobe for update

#define VS1063_CODEC_ADDRESS_GPIO_DDR   0xC017 // page 86
#define VS1063_CODEC_ADDRESS_I2S_CONFIG 0xC040 // page 86

// VS1063 codec's magic flag values according to datasheet
#define VS1063_CODEC_F_SM_LAYER12       0x0002 // page 44
#define VS1063_CODEC_F_SM_RESET         0x0004 // page 44
#define VS1063_CODEC_F_SM_CANCEL        0x0008 // page 44
#define VS1063_CODEC_F_SM_SDINEW        0x0800 // page 44
#define VS1063_CODEC_F_SM_ENCODE        0x1000 // page 44

#define VS1063_CODEC_F_SC_MULT_5_0X     0xE000 // page 48

#define VS1063_CODEC_F_PL_MO_EQ5_ENABLE 0x20   // page 77
#define VS1063_CODEC_F_EQ5_UPD_STROBE   0x01   // page 77

#define VS1063_CODEC_F_GPIO_DDR_192k    0xD0   // page 86

#define VS1063_CODEC_F_I2S_CONFIG_RESET 0x00   // page 86
#define VS1063_CODEC_F_I2S_CONFIG_192k  0x06   // page 86

/******************************************************************************
 * Low-Level hardware access functions
 *****************************************************************************/

UWORD ReadReg16( APTR amiGUS, ULONG offset );
ULONG ReadReg32( APTR amiGUS, ULONG offset );
UWORD ReadCodecSPI( APTR amiGUS, UWORD SPIregister );  
UWORD ReadVS1063Mem( APTR amiGUS, UWORD address );

VOID WriteReg16( APTR amiGUS, ULONG offset, UWORD value );
VOID WriteReg32( APTR amiGUS, ULONG offset, ULONG value );
VOID WriteCodecSPI( APTR amiGUS, UWORD SPIregister, UWORD SPIvalue );
VOID WriteVS1063Mem( APTR amiGUS, UWORD address, UWORD value );

VOID InitVS1063Codec( APTR amiGUS );

/**
 * Initializes and sets the equalizer to settings.
 *
 * @param amiGUS         Pointer to AmiGUS Codec part card base.
 * @param enable         TRUE to enable the equalizer,
 *                       FALSE to disable it.
 * @param settings       Nine element WORD array, with alternatingly:
 *                       0 - band 1 setting - between -32 and +32,
 *                       1 - border frequency 1 -   20 -   150Hz,
 *                       2 - band 2 setting - between -32 and +32,
 *                       3 - border frequency 2 -   50 -  1000Hz,
 *                       4 - band 3 setting - between -32 and +32,
 *                       5 - border frequency 3 - 1000 - 15000Hz,
 *                       6 - band 4 setting - between -32 and +32,
 *                       7 - border frequency 4 - 2000 - 15000Hz,
 *                       8 - band 5 setting - between -32 and +32.
 */
VOID InitVS1063Equalizer( APTR amiGUS, BOOL enable, const WORD * settings );

/**
 * Updates a single equalizer band to a desired setting.
 *
 * @param amiGUS         Pointer to AmiGUS Codec part card base.
 * @param equalizerLevel One equalizer level between
 *                       VS1063_CODEC_ADDRESS_EQ5_LEVEL1 and
 *                       VS1063_CODEC_ADDRESS_EQ5_LEVEL5.
 * @param value          A new equalizer setting for this frequency band or
 *                       level, between -32 and +32.
 */
VOID UpdateVS1063Equalizer( APTR amiGUS, UWORD equalizerLevel, WORD value );

/******************************************************************************
 * Low-Level hardware feature lookup tables
 *****************************************************************************/

extern const LONG AmiGUSSampleRates[ AMIGUS_PCM_SAMPLE_RATE_COUNT ];
extern const STRPTR AmiGUSOutputs[ AMIGUS_OUTPUTS_COUNT ];
extern const STRPTR AmiGUSInputs[ AMIGUS_INPUTS_COUNT ];
extern const UWORD AmiGUSInputFlags[ AMIGUS_INPUTS_COUNT ];
extern const WORD AmiGUSDefaultEqualizer[];
extern const WORD AmiGUSAmigaAmpEqualizer[];

#endif /* AMIGUS_HARDWARE_H */
