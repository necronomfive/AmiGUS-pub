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
 * and relay further for playback and recording.
 *
 * @param data Pointer to the handle address.
 *
 * @return 1 if the handle's card's interrupt was pending and handled,
 *         0 otherwise.
 */
ASM( LONG ) HandleInterruptNew( REG( d1, APTR data ));

#endif /* INTERRUPT_H */