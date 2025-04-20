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

#ifndef AMIGUS_CODEC_H
#define AMIGUS_CODEC_H

#include <exec/types.h>
#include <libraries/configvars.h>

/* Forward declaration here. */
struct AmiGUS_MHI_Handle;

LONG FindAmiGusCodec( struct ConfigDev ** device );
VOID StartAmiGusCodecPlayback( struct AmiGUS_MHI_Handle * handle );
VOID StopAmiGusCodecPlayback( struct AmiGUS_MHI_Handle * handle );
VOID SleepCodecTicks( APTR amiGUS, ULONG ticks );

#endif /* AMIGUS_CODEC_H */