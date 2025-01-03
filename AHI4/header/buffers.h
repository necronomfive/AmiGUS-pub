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

#ifndef BUFFERS_H
#define BUFFERS_H

#include <exec/types.h>

#include "SDI_compiler.h"

/**
 * Get an aligned buffer's size in bytes for holding a well defined fraction
 * of a second.
 *
 * @param sampleRate In samples per second, aka Hz, aka frequency by AHI.
 * @param secondFraction n in 1/n, defining how much of a second shall fit.
 * @param sampleSize In bytes per AHI sample, including stereo / mono.
 * @param multipleOf Buffer size requirement by the avisioned copy function.
 *
 * @return Desired buffer size in bytes.
 */
ULONG getBufferSize(
  LONG sampleRate,
  LONG secondFraction,
  UBYTE sampleSize,
  UBYTE multipleOf
);

/**
 * Get the required mixing / playback buffer size in bytes.
 *
 * @param sampleRate In samples per second, aka Hz, aka frequency by AHI.
 * @param sampleSize In bytes per AHI sample, independent from stereo and mono.
 * @param multipleOf Buffer size requirement by the avisioned copy function.
 * @param isStereo TRUE if stereo, FALSE if not.
 * @param isRealtime TRUE if the latency shall be 10ms or lower, FALSE if not.
 *
 * @return Desired buffer size in bytes.
 */
ULONG getBufferBytes(
  LONG sampleRate,
  UBYTE sampleSize,
  UBYTE multipleOf,
  UBYTE isStereo,
  UBYTE isRealtime );

/**
 * Get the required mixing / playback buffer in samples.
 *
 * @param bufferBytes Buffer size in bytes.
 * @param sampleSize In bytes per AHI sample, independent from stereo and mono.
 * @param isStereo TRUE if stereo, FALSE if not.
 *
 * @return Desired samples per buffer fill round.
 */
UWORD getBufferSamples(
  UWORD bufferBytes,
  UBYTE sampleSize,
  UBYTE isStereo );

/**
 * Returns an appropriately aligned BYTE size for
 * a requested number of samples for use with the selected
 * copy function and playback mode properies.
 *
 * @param ahiBufferSamples Sample count suggested by AHI.
 *
 * @return Byte size to be fed to AHI.
 */
ULONG AlignByteSizeForSamples( ULONG ahiBufferSamples );

/**
 * As AHI suggests to only support AHIST_S16S for recording,
 * everything is pretty fixed here, as sad as it is.
 * We will aim for a buffer size 
 * divisible by 8, 4 buffers per second, 4 bytes per sample.
 *
 * @param sampleRate Sample rate to use.
 *
 * @return Buffer byte size according to parameters above.
 */
ULONG getRecordingBufferSize( LONG sampleRate );

BOOL CreatePlaybackBuffers( VOID );
VOID DestroyPlaybackBuffers( VOID );

BOOL CreateRecordingBuffers( VOID );
VOID DestroyRecordingBuffers( VOID );

/**
 * Type of the copy functions provided here.
 */
typedef LONG ( ASM( * ) CopyFunctionType )(
  REG( d0, ULONG * ),
  REG( a0, ULONG * ) );

/**
 * so... copy functions needed...
 *    AHI    AmiGUS Required            x    resulting
 *    Sample Sample Alignment      HiFi Bit  Copy
 * ID Size   Size   Input Output   Mode Mode Function
 * 0  16      8      8     4 bytes       8   PlaybackCopy16to8
 * 1  16     16      4     4 bytes      16   PlaybackCopy16to16
 * 2  32      8     16     4 bytes x     8   PlaybackCopy32to8
 * 3  32     16      8     4 bytes x    16   PlaybackCopy32to16
 * 4  32     24     16    12 bytes x    24   PlaybackCopy32to24
 */
extern CopyFunctionType PlaybackCopyFunctionById[];

/**
 * Reads 2 LONGs, a, and b,
 * writes 1 LONG, 
 *   ( a & 0xff000000 )       |
 *   ( a & 0x0000ff00 ) <<  8 | 
 *   ( b & 0xff000000 ) >> 16 |
 *   ( b & 0x0000ff00 ) >>  8.
 * Used for 8bit non-HiFi modes.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 2 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM(LONG) PlaybackCopy16to8(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * Reads 1 LONG, writes the same 1 LONG.
 * Used for 16bit non-HiFi modes.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 1 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM(LONG) PlaybackCopy16to16(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * Reads 4 LONGs, a, b, c, and d,
 * writes 1 LONG, 
 *   ( a & 0xff000000 )       |
 *   ( b & 0xff000000 ) >>  8 | 
 *   ( c & 0xff000000 ) >> 16 |
 *   ( d & 0xff000000 ) >> 24.
 * Used for 8bit HiFi modes.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 2 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM(LONG) PlaybackCopy32to8(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * Reads 2 LONGs, a, and b,
 * writes 1 LONG, a & 0xffFF0000 | b >> 16.
 * Used for 16bit HiFi modes.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 2 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM(LONG) PlaybackCopy32to16(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * Reads 4 LONGs, a, b, c, and d,
 * writes 3 LONGs,
 * 1:         a & 0xffFFff00 | b >> 24, 
 * 2:  (b << 8) & 0xffFF0000 | c >> 16,
 * 3: (c << 16) & 0xFF000000 | d >> 8
 * Used for 24bit HiFi modes.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 12.
 */
ASM(LONG) PlaybackCopy32to24(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * so... MORE copy functions needed...
 *    AHI    AmiGUS Required            x    resulting
 *    Sample Sample Alignment      HiFi Bit  Copy
 * ID Size   Size   Input Output   Mode Mode Function
 * 0  32      8      4    16 bytes       8   RecordingCopy8Mto16S
 * 1  32     16      4     8 bytes       8   RecordingCopy8Sto16S
 * 2  32     16      4     8 bytes  16, 24   RecordingCopy16Mto16S
 * 3  32     32      4     4 bytes  16, 24   RecordingCopy16Sto16S
 */
extern CopyFunctionType RecordingCopyFunctionById[];

/**
 * Reads 1 LONG aka 4 samples in 8Bit Mono from AmiGUS
 * and prepares it for AHI 16Bit Stereo Non-HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM( LONG ) RecordingCopy8Mto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

/**
 * Reads 1 LONG aka 2 samples in 8Bit Stereo from AmiGUS
 * and prepares it for AHI 16Bit Stereo Non-HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM( LONG ) RecordingCopy8Sto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

/**
 * Reads 1 LONG aka 2 samples in 16Bit Mono from AmiGUS
 * and prepares it for AHI 16Bit Stereo Non-HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM( LONG ) RecordingCopy16Mto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

/**
 * Reads 1 LONG aka 1 sample in 16Bit Stereo from AmiGUS
 * and prepares it for AHI 16Bit Stereo Non-HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM( LONG ) RecordingCopy16Sto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

#endif /* BUFFERS_H */
