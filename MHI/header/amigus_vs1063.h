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

VOID InitVS1063Equalizer( APTR amiGUS, BOOL enable, const WORD * settings );

VOID UpdateVS1063Equalizer( APTR amiGUS, UWORD equalizerLevel, WORD value );

ULONG GetVS1063EndFill( APTR amiGUS );
  
VOID CancelVS1063Playback( APTR amiGUS );

VOID ResetVS1063( APTR amiGUS );
