#ifndef COPY_FUNCTIONS_H
#define COPY_FUNCTIONS_H

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

/*
 * so... copy functions needed...
 *  AHI   AmiGUS   Required     resulting
 * Sample Sample  Alignment        Copy
 *  Size   Size  Input Output   Function
 *    8      8     4   4  bytes PlainLongCopy
 *   16     16     4   4  bytes PlainLongCopy
 *   32     16     8   4  bytes Shift16LongCopy
 *   32     24    16   12 bytes Merge24LongCopy
 */

/**
 * Reads 1 LONG, writes the same 1 LONG.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 1 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM(LONG) SAVEDS PlainLongCopy(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * Reads 2 LONGs, a, and b,
 * writes 1 LONG, a & 0xffFF0000 | b >> 16.
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 2 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 4.
 */
ASM(LONG) SAVEDS Shift16LongCopy(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

/**
 * Reads 4 LONGs, a, b, c, and d,
 * writes 3 LONGs,
 * 1:         a & 0xffFFff00 | b >> 24, 
 * 2:  (b << 8) & 0xffFF0000 | c >> 16,
 * 3: (c << 16) & 0xFF000000 | d >> 8
 *
 * @param[in]      bufferBase  Buffer's base address
 * @param[in, out] bufferIndex Index applied onto the buffer already,
 *                             increased by 4 as counting LONGs here.
 *
 * @return Number of bytes written, i.e. 12.
 */
ASM(LONG) SAVEDS Merge24LongCopy(
  REG(d0, ULONG *bufferBase), 
  REG(a0, ULONG *bufferIndex) );

#endif /* COPY_FUNCTIONS_H */
