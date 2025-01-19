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

/**
 * so... playback copy functions needed...
 *    AHI    AmiGUS Required            x    resulting
 *    Sample Sample Alignment      HiFi Bit  Copy
 * ID Size   Size   Input Output   Mode Mode Function
 * 0  16      8      8     4 bytes       8   PlaybackCopy16to8
 * 1  16     16      4     4 bytes      16   PlaybackCopy16to16
 * 2  32      8     16     4 bytes x     8   PlaybackCopy32to8
 * 3  32     16      8     4 bytes x    16   PlaybackCopy32to16
 * 4  32     24     16    12 bytes x    24   PlaybackCopy32to24
 */
extern const CopyFunctionType PlaybackCopyFunctionById[ 5 ];

/**
 * so... MORE copy functions needed... for recording...
 *    AHI    AmiGUS Required            x    resulting
 *    Sample Sample Alignment      HiFi Bit  Copy
 * ID Size   Size   Input Output   Mode Mode Function
 * 0  32      8      4    16 bytes       8   RecordingCopy8Mto16S
 * 1  32     16      4     8 bytes       8   RecordingCopy8Sto16S
 * 2  32     16      4     8 bytes      16   RecordingCopy16Mto16S
 * 3  32     32      4     4 bytes      16   RecordingCopy16Sto16S
 * 4  64     24      12   16 bytes      24   RecordingCopy24Mto32S
 * 5  64     48      12   16 bytes      24   RecordingCopy24Sto32S
 */
extern const CopyFunctionType RecordingCopyFunctionById[ 6 ];

/**
 * Defines the AHI sample type to return for recording.
 */
extern const UBYTE RecordingSampleTypeById[ 6 ];

/**
 * Defines how the recording buffer must be aligned,
 * i.e. what multiple of bytes it need to hold to be
 * nicely filled with hardware samples from AmiGUS.
 */
extern const UBYTE RecordingSampleAlignmentById[ 6 ];


#endif /* COPIES_H */