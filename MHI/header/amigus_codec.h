/*
 * This file is part of the mhiAmiGUS.library.
 *
 * mhiAmiGUS.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_CODEC_H
#define AMIGUS_CODEC_H

/* Forward declaration here. */
struct AmiGUSBase;

LONG FindAmiGusCodec( struct AmiGUSBase *amiGUSBase );
VOID StartAmiGusCodecPlayback( VOID );
VOID StopAmiGusCodecPlayback( VOID );

#endif /* AMIGUS_CODEC_H */