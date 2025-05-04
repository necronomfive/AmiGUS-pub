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

#ifndef SDI_MHI_PROTOS_H
#define SDI_MHI_PROTOS_H

// TO NEVER BE USED OUTSIDE THE LIBRARY CODE !!!

#include <exec/types.h>
#include <exec/tasks.h>

#include "SDI_compiler.h"

/* Forward declaration here. */
struct AmiGUS_MHI;
struct AmiGUS_MHI_Handle;

/******************************************************************************
 * MHI library interface functions rewritten into SDI_compiler macros,
 * thereby making them compiler agnostic and 
 * adapting them for AmiGUS library internal usage.
 *
 * Detailed explanation see
 * https://aminet.net/driver/audio/mhi_dev.lha ->
 * MHI_dev/Autodoc/mhi.doc
 *****************************************************************************/

ASM( APTR ) SAVEDS MHIAllocDecoder(
  REG( a0, struct Task * task ),
  REG( d0, ULONG signal ),
  REG( a6, struct AmiGUS_MHI * base ));

ASM( VOID ) SAVEDS MHIFreeDecoder(
  REG( a3, struct AmiGUS_MHI_Handle * handle ),
  REG( a6, struct AmiGUS_MHI * base ));

ASM( BOOL ) SAVEDS MHIQueueBuffer(
  REG( a3, struct AmiGUS_MHI_Handle * handle ),
  REG( a0, APTR buffer ),
  REG( d0, ULONG size),
  REG( a6, struct AmiGUS_MHI * base ));

ASM( APTR ) SAVEDS MHIGetEmpty(
  REG( a3, struct AmiGUS_MHI_Handle * handle ),
  REG( a6, struct AmiGUS_MHI * base ));

ASM( UBYTE ) SAVEDS MHIGetStatus(
  REG( a3, struct AmiGUS_MHI_Handle * handle ),
  REG( a6, struct AmiGUS_MHI * base ));

ASM( VOID ) SAVEDS MHIPlay(
  REG( a3, struct AmiGUS_MHI_Handle * handle ),
  REG( a6, struct AmiGUS_MHI * base ));

ASM( VOID ) SAVEDS MHIStop(
  REG( a3, struct AmiGUS_MHI_Handle * handle ),
  REG( a6, struct AmiGUS_MHI * base ));

ASM( VOID ) SAVEDS MHIPause(
  REG( a3, struct AmiGUS_MHI_Handle * handle ),
  REG( a6, struct AmiGUS_MHI * base ));

ASM( ULONG ) SAVEDS MHIQuery(
  REG( d1, ULONG query ),
  REG( a6, struct AmiGUS_MHI * base ));

ASM( VOID ) SAVEDS MHISetParam(
  REG( a3, struct AmiGUS_MHI_Handle * handle ),
  REG( d0, UWORD param ),
  REG( d1, ULONG value ),
  REG( a6, struct AmiGUS_MHI * base ));

#endif /* SDI_MHI_PROTOS_H */
