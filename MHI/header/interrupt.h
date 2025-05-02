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

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "exec/types.h"
#include "SDI_compiler.h"

/**
 * Creates the interupt handler for the MHI driver library.
 *
 * @return ENoError = 0 if successful,
 *         error code otherwise.
 */
LONG CreateInterruptHandler( VOID );

/**
 * Destroys the interupt handler of the MHI driver library.
 */
VOID DestroyInterruptHandler( VOID );

/**
 * Fills the codec's playback buffer with however many more encoded data
 * is available, handling buffer exchanges, end of buffers, underruns, etc.
 *
 * @param handle Pointer to the client's AmiGUS codec state handle.
 */
VOID FillCodecBuffer( struct AmiGUS_MHI_Handle * handle );

#endif /* INTERRUPT_H */
