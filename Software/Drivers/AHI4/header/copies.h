/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COPIES_H
#define COPIES_H

#include <exec/types.h>

#include "SDI_compiler.h"

/**
 * Type of the copy functions provided here.
 */
typedef LONG ( ASM( * ) CopyFunctionType )(
  REG( d0, ULONG * ),
  REG( a0, ULONG * ) );

/*
 * Declaring the copy functions here - but the real external interfaces
 * are the playback and recording AHI mode lookups in ahi_modes.h .
 */

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
 * Reads 1 LONG aka 4 samples in 8Bit Mono from AmiGUS
 * and prepares it for AHI 16Bit Stereo Non-HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes read, i.e. 4.
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
 * @return Number of bytes read, i.e. 4.
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
 * @return Number of bytes read, i.e. 4.
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
 * @return Number of bytes read, i.e. 4.
 */
ASM( LONG ) RecordingCopy16Sto16S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

/**
 * Reads 3 LONG aka 4 samples in 24Bit Mono from AmiGUS
 * and prepares it for AHI 32Bit Stereo HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 8 as counting LONGs here.
 *
 * @return Number of bytes read, i.e. 6.
 */
ASM( LONG ) RecordingCopy24Mto32S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

/**
 * Reads 6 LONG aka 8 samples in 24Bit Stereo from AmiGUS
 * and prepares it for AHI 32Bit Stereo HiFi consumption.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 8 as counting LONGs here.
 *
 * @return Number of bytes read, i.e. 12.
 */
ASM( LONG ) RecordingCopy24Sto32S(
  REG( d0, ULONG *bufferBase ),
  REG( a0, ULONG *bufferIndex ));

#endif /* COPIES_H */