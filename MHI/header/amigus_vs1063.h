/*
 * This file is part of the mhiamigus.library.
 *
 * mhiamigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiamigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiamigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_VS1063_H
#define AMIGUS_VS1063_H

#include <exec/types.h>

/******************************************************************************
 * VS1063 codec convenience functions.
 *****************************************************************************/

/**
 * Prepares the VS1063 codec chip for use on the AmiGUS card.
 * Sets the clock as needed and enables I2C interface at 192kHz sample rate.
 *
 * @param amiGUS         Pointer to the AmiGUS codec's register bank.
 */
VOID InitVS1063Codec( APTR amiGUS );

/**
 * Initializes and sets the equalizer to settings.
 *
 * @param amiGUS         Pointer to the AmiGUS codec's register bank.
 * @param enable         TRUE to enable the equalizer,
 *                       FALSE to disable it.
 * @param settings       Nine element WORD array, with alternatingly:
 *                       0 - band 1 setting - between -32 and +32,
 *                       1 - border frequency 1 -   20 -   150Hz,
 *                       2 - band 2 setting - between -32 and +32,
 *                       3 - border frequency 2 -   50 -  1000Hz,
 *                       4 - band 3 setting - between -32 and +32,
 *                       5 - border frequency 3 - 1000 - 15000Hz,
 *                       6 - band 4 setting - between -32 and +32,
 *                       7 - border frequency 4 - 2000 - 15000Hz,
 *                       8 - band 5 setting - between -32 and +32.
 */
VOID InitVS1063Equalizer( APTR amiGUS, BOOL enable, const WORD * settings );

/**
 * Sets a single equalizer band to a desired setting.
 *
 * @param amiGUS         Pointer to the AmiGUS codec's register bank.
 * @param equalizerLevel One equalizer level between
 *                       VS1063_CODEC_ADDRESS_EQ5_LEVEL1 and
 *                       VS1063_CODEC_ADDRESS_EQ5_LEVEL5.
 * @param value          A new equalizer setting for this frequency band or
 *                       level, between -32 and +32.
 */
VOID SetVS1063Equalizer( APTR amiGUS, UWORD equalizerLevel, WORD value );

/**
 * Updates the codec's 5 hardware equalizer band settings
 * from a 10 band input and 1 gain value.
 * In other words: adapts the AmigaAmp UI settings to the chip's capabilities.
 *
 * @param amiGUS         Pointer to the AmiGUS codec's register bank.
 * @param values         Array of 10 band equilizer input settings at
 *                       indices 0 to 9 and 1 gain setting at index 10
 *                       in percent values from 0 (maximum damping) to
 *                       100 (maximum gain).
 */
VOID UpdateVS1063Equalizer( APTR amiGUS, UBYTE values[ 11 ]);

/**
 * Updates the codec's hardware volume setting
 * from one volume and one panning value.
 * In other words: adapts the AmigaAmp UI settings to the chip's capabilities.
 *
 * @param amiGUS         Pointer to the AmiGUS codec's register bank.
 * @param volume         Meant to be between 0 = muted and 100 = -0dB
 * @param panning        Meant to be between 0 = left and 100 = right.
 */
VOID UpdateVS1063VolumePanning( APTR amiGUS, UBYTE volume, UBYTE panning );

/**
 * Fetches the codec dependent "end fill" byte from the codec chip and
 * quadruples it into the ULONG returned. Meant to be used at the end of
 * each song to gently close the song without disturbing noises.
 *
 * @param amiGUS         Pointer to the AmiGUS codec's register bank.
 */
ULONG GetVS1063EndFill( APTR amiGUS );

/**
 * Correctly cancels the codec's playback during a song.
 * Harder than it sounds...
 *
 * @param amiGUS         Pointer to the AmiGUS codec's register bank.
 */
VOID CancelVS1063Playback( APTR amiGUS );

#endif /* AMIGUS_VS1063_H */
