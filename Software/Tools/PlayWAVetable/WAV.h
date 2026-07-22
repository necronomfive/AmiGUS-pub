/*
 * This file is part of the AmiGUS PlayWAVetable Utility.
 *
 * AmiGUS PlayWAVetable Utility is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS PlayWAVetable Utility is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AmiGUS PlayWAVetable Utility.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WAV_H
#define WAV_H

#include <dos/dos.h>
#include <exec/types.h>

/**
 * WAV_BUFFER_SIZE is approximately 8k and nicely devisable by 
 * 1 for 8bit Mono
 * 2 for 8bit Stereo and 16bit Mono
 * 3 for 24bit Mono
 * 4 for 16bit Stereo and
 * 6 for 24bit Stereo
 * 
 * ... and therefore, a nice default.
 */
#define WAV_BUFFER_SIZE              8184

struct wav {

  BPTR  wav_File;
  UWORD wav_Channels;
  ULONG wav_SampleRate;
  ULONG wav_SampleBits;
  ULONG wav_DataSize;
  LONG  wav_DataOffset;
};

/*****************************************************
 * WAV file loading and parsing functions below.
 *****************************************************/

/**
 * Opens the WAV file and parses the header, filling the provided WAV struct.
 *
 * @param wav Pointer to the WAV struct to be filled.
 * @param filename Path to the WAV file to be opened.
 *
 * Returns NoError = 0 on success,
 * or an error code on failure.
 */
LONG OpenWav( struct wav * wav, STRPTR filename );

/**
 * Closes the WAV file and frees any allocated resources.
 *
 * @param wav Pointer to the WAV struct to be cleaned up.
 */
VOID CloseWav( struct wav * wav );

/**
 * Reads the next chunk of sample data in little endian format
 * into the WAV's internal buffer.
 *
 * @param wav Pointer to the WAV struct to be used.
 * @param data Pointer to where the data chunk should be stored.
 * @param size Size of the data chunk
 *
 * @return Number of bytes read.
 */
LONG ReadWavChunkLE( struct wav * wav, APTR data, LONG size );

/**
 * Reads the next chunk of sample data in big endian format
 * into the WAV's internal buffer.
 *
 * @param wav Pointer to the WAV struct to be used.
 * @param data Pointer to where the data chunk should be stored.
 * @param size Size of the data chunk
 *
 * @return Number of bytes read.
 */
LONG ReadWavChunkBE( struct wav * wav, APTR data, LONG size );

#endif /* WAV_H */
