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

BOOL CreatePlaybackBuffers( ULONG bufferSize );
VOID DestroyPlaybackBuffers( VOID );

BOOL CreateRecordingBuffers( VOID );
VOID DestroyRecordingBuffers( VOID );

#endif /* BUFFERS_H */
