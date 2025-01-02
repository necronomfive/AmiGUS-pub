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
#define AMIGUS_MP3_PRODUCT_ID            18

/* AmiGUS PCM Registers */
#define AMIGUS_PCM_MAIN_INT_CONTROL      0x00
#define AMIGUS_PCM_MAIN_INT_ENABLE       0x02

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
#define AMIGUS_INT_F_PLAY_FIFO_EMPTY     0x0001
#define AMIGUS_INT_F_PLAY_FIFO_FULL      0x0002
#define AMIGUS_INT_F_PLAY_FIFO_WATERMARK 0x0004
#define AMIGUS_INT_F_SPI_TRANSFER_FINISH 0x0008
#define AMIGUS_INT_F_REC_FIFO_EMPTY      0x0010
#define AMIGUS_INT_F_REC_FIFO_FULL       0x0020
#define AMIGUS_INT_F_REC_FIFO_WATERMARK  0x0040
#define AMIGUS_INT_F_TIMER_ENABLE        0x4000
#define AMIGUS_INT_F_SET                 0x8000
#define AMIGUS_INT_F_CLEAR               0x0000

/* AmiGUS PCM Playback Sample Formats */
#define AMIGUS_PCM_S_PLAY_MONO_8BIT      0x0000
#define AMIGUS_PCM_S_PLAY_STEREO_8BIT    0x0001
#define AMIGUS_PCM_S_PLAY_MONO_16BIT     0x0002
#define AMIGUS_PCM_S_PLAY_STEREO_16BIT   0x0003
#define AMIGUS_PCM_S_PLAY_MONO_24BIT     0x0004
#define AMIGUS_PCM_S_PLAY_STEREO_24BIT   0x0005
#define AMIGUS_PCM_S_PLAY_FORMAT_COUNT   6
/* AMIGUS PCM Playback Sample Format Flags */
#define AMIGUS_PCM_S_PLAY_F_SWAP_CH      0x0008
#define AMIGUS_PCM_S_PLAY_F_LITTLE_END   0x0010
#define AMIGUS_PCM_S_PLAY_FLAG_UNSIGNED  0x0020

/* AmiGUS PCM Recording Sample Formats */
#define AMIGUS_PCM_S_REC_MONO_8BIT       0x0006
#define AMIGUS_PCM_S_REC_STEREO_8BIT     0x0000
#define AMIGUS_PCM_S_REC_MONO_16BIT      0x0007
#define AMIGUS_PCM_S_REC_STEREO_16BIT    0x0001
#define AMIGUS_PCM_S_REC_MONO_24BIT      Not Available
#define AMIGUS_PCM_S_REC_STEREO_24BIT    Not Available
#define AMIGUS_PCM_S_REC_FORMAT_COUNT    4

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
#define AMIGUS_PCM_FIFO_RESET            0x0000

/* FIFO size */
#define AMIGUS_PCM_PLAY_FIFO_BYTES       8192
#define AMIGUS_PCM_PLAY_FIFO_WORDS       4096
#define AMIGUS_PCM_PLAY_FIFO_LONGS       2048

#define AMIGUS_PCM_REC_FIFO_BYTES        2048
#define AMIGUS_PCM_REC_FIFO_WORDS        1024
#define AMIGUS_PCM_REC_FIFO_LONGS        512

#define AMIGUS_OUTPUTS_COUNT             1
#define AMIGUS_INPUTS_COUNT              5

/******************************************************************************
 * Low-Level hardware access functions
 *****************************************************************************/

UWORD ReadReg16( APTR amiGUS, ULONG offset );
VOID WriteReg16( APTR amiGUS, ULONG offset, UWORD value );
ULONG ReadReg32( APTR amiGUS, ULONG offset );
VOID WriteReg32( APTR amiGUS, ULONG offset, ULONG value );

/******************************************************************************
 * Low-Level hardware feature lookup tables
 *****************************************************************************/

extern const LONG AmiGUSSampleRates[ AMIGUS_PCM_SAMPLE_RATE_COUNT ];
extern const STRPTR AmiGUSOutputs[ AMIGUS_OUTPUTS_COUNT ];
extern const STRPTR AmiGUSInputs[ AMIGUS_INPUTS_COUNT ];
extern const UWORD AmiGUSSampleSizes[ AMIGUS_PCM_S_PLAY_FORMAT_COUNT ];

#endif /* AMIGUS_HARDWARE_H */
