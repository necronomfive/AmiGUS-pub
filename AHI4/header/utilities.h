#ifndef UTILITIES_H
#define UTILITIES_H

#include <exec/types.h>

#include "SDI_compiler.h"

/**
 * Function to return the greatest common denominator of two numbers.
 *
 * @param a The first number.
 * @param b The other number.
 *
 * @return The result.
 */
UWORD gcd( UWORD a, UWORD b );

/** 
 * Function to return the least common multiple of two numbers.
 *
 * @param a The first number.
 * @param b The other number.
 *
 * @return The result.
 */
ULONG lcm( ULONG a, ULONG b );

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
UWORD getBufferBytes(
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
 * Alignment requirements of the copy functions encrypted into BYTE masks.
 * Order follows the same as CopyFunctionById[].
 */
extern const ULONG CopyFunctionRequirementById[];

/**
 * Returns an appropriately aligned/sized sample count for
 * a requested number of samples for use with the selected
 * copy function and playback mode properies.
 *
 * @param ahiBuffSamples Sample count suggested by AHI.
 *
 * @return Adapted sample count to be fed to AHI.
 */
ULONG alignBufferSamples( ULONG ahiBuffSamples );

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
 * 0  16      8      8     4 bytes       8   Copy16to8
 * 1  16     16      4     4 bytes      16   Copy16to16
 * 2  32      8     16     4 bytes x     8   Copy32to8
 * 3  32     16      8     4 bytes x    16   Copy32to16
 * 4  32     24     16    12 bytes x    24   Copy32to24
 */
extern CopyFunctionType CopyFunctionById[];

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
ASM(LONG) Copy16to8(
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
ASM(LONG) Copy16to16(
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
ASM(LONG) Copy32to8(
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
ASM(LONG) Copy32to16(
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
ASM(LONG) Copy32to24(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

#endif /* UTILITIES_H */
