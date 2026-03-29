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

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "exec/types.h"
#include "SDI_compiler.h"

/******************************************************************************
 * Interrupt functions.
 *****************************************************************************/

/**
 * Interrupt handler function,
 * checking the status of the relevant AmiGUS card, and
 * and filling their buffers accordingly.
 *
 * @param data Pointer to the handle address.
 *
 * @return 1 if the handle's card's interrupt was pending and handled,
 *         0 otherwise.
 */
ASM( LONG ) HandleInterruptNew( REG( d1, APTR data ));

/**
 * Fills the codec's playback buffer with however many more encoded data
 * is available, handling buffer exchanges, end of buffers, underruns, etc.
 *
 * @param handle Pointer to the client's AmiGUS codec state handle.
 */
VOID FillCodecBuffer( struct AmiGUS_MHI_Handle * handle );

#endif /* INTERRUPT_H */
