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

#ifndef AMIGUS_H
#define AMIGUS_H

/* AmiGUS Zorro IDs */

#define AMIGUS_MANUFACTURER_ID  2782

#define AMIGUS_MAIN_PRODUCT_ID  16
#define AMIGUS_HAGEN_PRODUCT_ID 17
#define AMIGUS_MP3_PRODUCT_ID   18

/* AmiGUS Main Registers */

#define AMIGUS_MAIN_INT_CONTROL     0x00
#define AMIGUS_MAIN_INT_ENABLE      0x02
#define AMIGUS_MAIN_SAMPLE_FORMAT   0x04
#define AMIGUS_MAIN_SAMPLE_RATE     0x06
#define AMIGUS_MAIN_FIFO_RESET      0x08
#define AMIGUS_MAIN_FIFO_WATERMARK  0x0a
#define AMIGUS_MAIN_FIFO_WRITE      0x0c
#define AMIGUS_MAIN_FIFO_USAGE      0x10

/* AmiGUS Main Interrupt Flags */
#define AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY       0x0001
#define AMIGUS_INT_FLAG_PLAYBACK_FIFO_FULL        0x0002
#define AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK   0x0004
#define AMIGUS_INT_FLAG_SPI_TRANSFER_FINISH       0x0008
#define AMIGUS_INT_FLAG_RECORD_FIFO_EMPTY         0x0010
#define AMIGUS_INT_FLAG_RECORD_FIFO_FULL          0x0020
#define AMIGUS_INT_FLAG_RECORD_FIFO_WATERMARK     0x0040
#define AMIGUS_INT_FLAG_TIMER_ENABLE              0x4000
#define AMIGUS_INT_FLAG_MASK_SET                  0x8000
#define AMIGUS_INT_FLAG_MASK_CLEAR                0x0000

/* AmiGUS Main Sample Formats */
#define AMIGUS_SAMPLE_FORMAT_MONO_8BIT            0x0000
#define AMIGUS_SAMPLE_FORMAT_STEREO_8BIT          0x0001
#define AMIGUS_SAMPLE_FORMAT_MONO_16BIT           0x0002
#define AMIGUS_SAMPLE_FORMAT_STEREO_16BIT         0x0003
#define AMIGUS_SAMPLE_FORMAT_MONO_24BIT           0x0004
#define AMIGUS_SAMPLE_FORMAT_STEREO_24BIT         0x0005
/* AMIGUS Main Sample Format Flags */
#define AMIGUS_SAMPLE_FORMAT_FLAG_SWAP_CHANNELS   0x0008
#define AMIGUS_SAMPLE_FORMAT_FLAG_LITTLE_ENDIAN   0x0010
#define AMIGUS_SAMPLE_FORMAT_FLAG_UNSIGNED        0x0020

/* AmiGUS Main Sample Rates */
#define AMIGUS_SAMPLE_RATE_8000                   0x0000
#define AMIGUS_SAMPLE_RATE_11025                  0x0001
#define AMIGUS_SAMPLE_RATE_16000                  0x0002
#define AMIGUS_SAMPLE_RATE_22050                  0x0003
#define AMIGUS_SAMPLE_RATE_24000                  0x0004
#define AMIGUS_SAMPLE_RATE_32000                  0x0005
#define AMIGUS_SAMPLE_RATE_44100                  0x0006
#define AMIGUS_SAMPLE_RATE_48000                  0x0007
#define AMIGUS_SAMPLE_RATE_96000                  0x0008
/* AmiGUS Main Sample Rate Flags */
#define AMIGUS_SAMPLE_RATE_FLAG_INTERPOLATION     0x4000
#define AMIGUS_SAMPLE_RATE_FLAG_ENABLE            0x8000
#define AMIGUS_SAMPLE_RATE_FLAG_DISABLE           0x0000

/* FIFO Reset */
#define AMIGUS_FIFO_RESET                         0x0000

#endif
