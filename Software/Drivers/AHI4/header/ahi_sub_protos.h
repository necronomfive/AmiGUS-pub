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

#ifndef AHI4_SUB_PROTOS_H
#define AHI4_SUB_PROTOS_H

#if defined (__VBCC__)
#pragma dontwarn 61
#endif

#include <libraries/ahi_sub.h>

#if defined (__VBCC__)
#pragma popwarn
#endif

#include "compiler_extras.h"

/* Basic functions */

ULONG __ASM__ __SAVE_DS__ AHIsub_AllocAudio(
  __REG__( a1, struct TagItem * aTagList ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

VOID __ASM__ __SAVE_DS__ AHIsub_FreeAudio(
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

VOID __ASM__ __SAVE_DS__ AHIsub_Disable(
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

VOID __ASM__ __SAVE_DS__ AHIsub_Enable(
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

ULONG __ASM__ __SAVE_DS__ AHIsub_Start(
  __REG__( d0, ULONG aFlags ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

VOID __ASM__ __SAVE_DS__ AHIsub_Update(
  __REG__( d0, ULONG aFlags ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

VOID __ASM__ __SAVE_DS__ AHIsub_Stop(
  __REG__( d0, ULONG aFlags ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

/* Acceleration functions */

ULONG __ASM__ __SAVE_DS__ AHIsub_SetVol(
  __REG__( d0, UWORD aChannel ),
  __REG__( d1, Fixed aVolume ),
  __REG__( d2, sposition aPan ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl ),
  __REG__( d3, ULONG aFlags )
);

ULONG __ASM__ __SAVE_DS__ AHIsub_SetFreq(
  __REG__( d0, UWORD aChannel ),
  __REG__( d1, ULONG aFreq ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl ),
  __REG__( d2, ULONG aFlags )
);

ULONG __ASM__ __SAVE_DS__ AHIsub_SetSound(
  __REG__( d0, UWORD aChannel ),
  __REG__( d1, UWORD aSound ),
  __REG__( d2, ULONG aOffset ),
  __REG__( d3, LONG aLength ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl ),
  __REG__( d4, ULONG aFlags )
);

ULONG __ASM__ __SAVE_DS__ AHIsub_SetEffect(
  __REG__( a0, APTR aEffect ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

ULONG __ASM__ __SAVE_DS__ AHIsub_LoadSound(
  __REG__( d0, UWORD aSound ),
  __REG__( d1, ULONG aType ),
  __REG__( a0, APTR aInfo ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

ULONG __ASM__ __SAVE_DS__ AHIsub_UnloadSound(
  __REG__( d0, UWORD aSound ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

/* Query functions */

LONG __ASM__ __SAVE_DS__ AHIsub_GetAttr(
  __REG__( d0, ULONG aAttribute ),
  __REG__( d1, LONG aArgument ),
  __REG__( d2, LONG aDefault ),
  __REG__( a1, struct TagItem* aTagList ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

/* Mixer functions */

LONG __ASM__ __SAVE_DS__ AHIsub_HardwareControl(
  __REG__( d0, ULONG aAttribute ),
  __REG__( d1, LONG aArgument ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
);

#endif /* AHI4_SUB_PROTOS_H */
