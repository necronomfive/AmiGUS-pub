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

struct wav {

  BPTR  wav_File;
  APTR  wav_Buffer;
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
 *
 * @return Number of bytes read.
 */
LONG ReadChunkLE( struct wav * wav );

/**
 * Reads the next chunk of sample data in big endian format
 * into the WAV's internal buffer.
 *
 * @param wav Pointer to the WAV struct to be used.
 *
 * @return Number of bytes read.
 */
LONG ReadChunkBE( struct wav * wav );

#endif /* WAV_H */
