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

#ifndef AIFF_H
#define AIFF_H

#include <dos/dos.h>
#include <exec/types.h>

struct aiff {

  BPTR  aiff_File;
  APTR  aiff_Buffer;
  UWORD aiff_Channels;
  ULONG aiff_SampleRate;
  ULONG aiff_SampleBits;
  ULONG aiff_DataSize;
  LONG  aiff_DataOffset;
};

/*****************************************************
 * AIFF file loading and parsing functions below.
 *****************************************************/

/**
 * Opens the AIFF file and parses the header, filling the provided AIFF struct.
 *
 * @param aiff Pointer to the AIFF struct to be filled.
 * @param filename Path to the AIFF file to be opened.
 *
 * Returns NoError = 0 on success,
 * or an error code on failure.
 */
LONG OpenAiff( struct aiff * aiff, STRPTR filename );

/**
 * Closes the AIFF file and frees any allocated resources.
 *
 * @param aiff Pointer to the AIFF struct to be cleaned up.
 */
VOID CloseAiff( struct aiff * aiff );

/**
 * Reads the next chunk of sample data in little endian format
 * into the AIFF's internal buffer.
 *
 * @param aiff Pointer to the AIFF struct to be used.
 *
 * @return Number of bytes read.
 */
LONG ReadAiffChunkLE( struct aiff * aiff );

/**
 * Reads the next chunk of sample data in big endian format
 * into the AIFF's internal buffer.
 *
 * @param aiff Pointer to the AIFF struct to be used.
 *
 * @return Number of bytes read.
 */
LONG ReadAiffChunkBE( struct aiff * aiff );

#endif /* AIFF_H */
