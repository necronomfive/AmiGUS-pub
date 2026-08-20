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

#ifndef MHI_PROTOS_H
#define MHI_PROTOS_H

// TO NEVER BE USED OUTSIDE THE LIBRARY CODE !!!

#include <exec/types.h>
#include <exec/tasks.h>

#include "compiler_extras.h"

/* Forward declaration here if needed. */
#ifndef AMIGUS_MHI_H

struct AmiGUS_MHI;
struct AmiGUS_MHI_Handle;

#endif /* AMIGUS_MHI_H */

/******************************************************************************
 * MHI library interface functions rewritten into SDI_compiler macros,
 * thereby making them compiler agnostic and 
 * adapting them for AmiGUS library internal usage.
 *
 * Detailed explanation see
 * https://aminet.net/driver/audio/mhi_dev.lha ->
 * MHI_dev/Autodoc/mhi.doc
 *****************************************************************************/

APTR __ASM__ __SAVE_DS__ MHIAllocDecoder(
  __REG__( a0, struct Task * task ),
  __REG__( d0, ULONG signal ),
  __REG__( a6, struct AmiGUS_MHI * base )
);

VOID __ASM__ __SAVE_DS__ MHIFreeDecoder(
  __REG__( a3, struct AmiGUS_MHI_Handle * handle ),
  __REG__( a6, struct AmiGUS_MHI * base )
);

BOOL __ASM__ __SAVE_DS__ MHIQueueBuffer(
  __REG__( a3, struct AmiGUS_MHI_Handle * handle ),
  __REG__( a0, APTR buffer ),
  __REG__( d0, ULONG size),
  __REG__( a6, struct AmiGUS_MHI * base )
);

APTR __ASM__ __SAVE_DS__ MHIGetEmpty(
  __REG__( a3, struct AmiGUS_MHI_Handle * handle ),
  __REG__( a6, struct AmiGUS_MHI * base )
);

UBYTE __ASM__ __SAVE_DS__ MHIGetStatus(
  __REG__( a3, struct AmiGUS_MHI_Handle * handle ),
  __REG__( a6, struct AmiGUS_MHI * base )
);

VOID __ASM__ __SAVE_DS__ MHIPlay(
  __REG__( a3, struct AmiGUS_MHI_Handle * handle ),
  __REG__( a6, struct AmiGUS_MHI * base )
);

VOID __ASM__ __SAVE_DS__ MHIStop(
  __REG__( a3, struct AmiGUS_MHI_Handle * handle ),
  __REG__( a6, struct AmiGUS_MHI * base )
);

VOID __ASM__ __SAVE_DS__ MHIPause(
  __REG__( a3, struct AmiGUS_MHI_Handle * handle ),
  __REG__( a6, struct AmiGUS_MHI * base )
);

ULONG __ASM__ __SAVE_DS__ MHIQuery(
  __REG__( d1, ULONG query ),
  __REG__( a6, struct AmiGUS_MHI * base )
);

VOID __ASM__ __SAVE_DS__ MHISetParam(
  __REG__( a3, struct AmiGUS_MHI_Handle * handle ),
  __REG__( d0, UWORD param ),
  __REG__( d1, ULONG value ),
  __REG__( a6, struct AmiGUS_MHI * base )
);

#endif /* MHI_PROTOS_H */
