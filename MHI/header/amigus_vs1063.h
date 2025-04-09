/*
 * This file is part of the mhiAmiGUS.library driver.
 *
 * mhiAmiGUS.library driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library driver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <exec/types.h>

UWORD ReadVS1063Mem( APTR amiGUS, UWORD address );

VOID WriteVS1063Mem( APTR amiGUS, UWORD address, UWORD value );

VOID InitVS1063Codec( APTR amiGUS );

/**
 * Initializes and sets the equalizer to settings.
 *
 * @param amiGUS         Pointer to AmiGUS Codec part card base.
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
 * Updates a single equalizer band to a desired setting.
 *
 * @param amiGUS         Pointer to AmiGUS Codec part card base.
 * @param equalizerLevel One equalizer level between
 *                       VS1063_CODEC_ADDRESS_EQ5_LEVEL1 and
 *                       VS1063_CODEC_ADDRESS_EQ5_LEVEL5.
 * @param value          A new equalizer setting for this frequency band or
 *                       level, between -32 and +32.
 */
VOID UpdateVS1063Equalizer( APTR amiGUS, UWORD equalizerLevel, WORD value );

VOID UpdateVS1063VolumePanning( APTR amiGUS, UBYTE volume, UBYTE panning );

ULONG GetVS1063EndFill( APTR amiGUS );
  
VOID CancelVS1063Playback( APTR amiGUS );

VOID ResetVS1063( APTR amiGUS );
