/*
 * This file is part of the mhiamigus.library.
 *
 * mhiamigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiamigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
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

/******************************************************************************
 * Codec convenience functions.
 *****************************************************************************/

/**
 * Starts playback of the AmiGUS codec as "owned" by the handle
 * provided from / to the player software.
 *
 * @param handle AmiGUS MHI handle as provided to the player by the driver
 *               and used to identify the player's MHI context.
 */
VOID StartAmiGusCodecPlayback( struct AmiGUS_MHI_Handle * handle );

/**
 * Stops playback of the AmiGUS codec as "owned" by the handle
 * provided from / to the player software.
 *
 * @param handle AmiGUS MHI handle as provided to the player by the driver
 *               and used to identify the player's MHI context.
 */
VOID StopAmiGusCodecPlayback( struct AmiGUS_MHI_Handle * handle );

/**
 * Busy waiting "sleep" method using the AmiGUS codec's high resolution timer.
 * Fine to use for less than a second, waste of CPU time otherwise.
 *
 * @param amiGUS Pointer to the AmiGUS codec's register bank.
 * @param ticks Time to wait in units of
 *              1 / AMIGUS_TIMER_CLOCK = 1 / 24576000 seconds,
 *              use MILLIS_PER_SECOND and MICROS_PER_SECOND for
 *              fine granular steering.
 */
VOID SleepCodecTicks( APTR amiGUS, ULONG ticks );

#endif /* AMIGUS_CODEC_H */
