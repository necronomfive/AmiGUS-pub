#ifndef COPY_FUNCTIONS_H
#define COPY_FUNCTIONS_H

#include <exec/types.h>

#include "SDI_compiler.h"

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
  REG(a0, ULONG *bufferIndex)
);

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
  REG(a0, ULONG *bufferIndex)
);

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
  REG(a0, ULONG *bufferIndex)
);

#endif /* COPY_FUNCTIONS_H */
