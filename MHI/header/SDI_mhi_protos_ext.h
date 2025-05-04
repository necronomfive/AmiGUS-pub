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

#ifndef SDI_MHI_PROTOS_EXT_H
#define SDI_MHI_PROTOS_EXT_H

// TO NEVER BE USED INSIDE THE LIBRARY CODE !!!

#include <exec/types.h>
#include <exec/tasks.h>

#include "SDI_compiler.h"

/******************************************************************************
 * MHI library interface functions rewritten into SDI_compiler macros,
 * making them compiler agnostic.
 * To be used when just using a random MHI driver library.
 *
 * Detailed explanation see
 * https://aminet.net/driver/audio/mhi_dev.lha ->
 * MHI_dev/Autodoc/mhi.doc
 *****************************************************************************/

ASM( APTR ) SAVEDS MHIAllocDecoder( REG( a0, struct Task * task ),
                                    REG( d0, ULONG signal ));

ASM( VOID ) SAVEDS MHIFreeDecoder( REG( a3, APTR handle ));

ASM( BOOL ) SAVEDS MHIQueueBuffer( REG( a3, APTR handle ),
                                   REG( a0, APTR buffer ),
                                   REG( d0, ULONG size ));

ASM( APTR ) SAVEDS MHIGetEmpty( REG( a3, APTR handle ));

ASM( UBYTE ) SAVEDS MHIGetStatus( REG( a3, APTR handle ));

ASM( VOID ) SAVEDS MHIPlay( REG( a3, APTR handle ));

ASM( VOID ) SAVEDS MHIStop( REG( a3, APTR handle ));

ASM( VOID ) SAVEDS MHIPause( REG( a3, APTR handle ));

ASM( ULONG ) SAVEDS MHIQuery( REG( d1, ULONG query ));

ASM( VOID ) SAVEDS MHISetParam( REG( a3, APTR handle ),
                                REG( d0, UWORD param ),
                                REG( d1, ULONG value ));

#endif /* SDI_MHI_PROTOS_EXT_H */
