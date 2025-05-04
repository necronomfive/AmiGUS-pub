/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_PCM_H
#define AMIGUS_PCM_H

LONG FindAmiGusPcm( struct AmiGUSBase *amiGUSBase );

VOID StartAmiGusPcmPlayback( VOID );
VOID StopAmiGusPcmPlayback( VOID );

VOID StartAmiGusPcmRecording( VOID );
VOID StopAmiGusPcmRecording( VOID );

#endif /* AMIGUS_PCM_H */