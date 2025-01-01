/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SDI_AHI4_PROTOS_H
#define SDI_AHI4_PROTOS_H

#if defined (__VBCC__)
#pragma dontwarn 61
#endif

#include <libraries/ahi_sub.h>

#if defined (__VBCC__)
#pragma popwarn
#endif

#include <SDI_compiler.h>

#if defined(__VBCC__)
  #define AMIGA_INTERRUPT __amigainterrupt
#elif defined(__SASC)
  #define AMIGA_INTERRUPT __interrupt
#endif

/* Basic functions */

ASM(ULONG) SAVEDS AHIsub_AllocAudio(
//  REG(a6, struct Library* aBase),
  REG(a1, struct TagItem* aTagList),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
);

ASM(void) SAVEDS AHIsub_FreeAudio(
  REG(a6, struct Library* aBase),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
);

ASM(void) SAVEDS AHIsub_Disable(
  REG(a6, struct Library* aBase),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
);

ASM(void) SAVEDS AHIsub_Enable(
  REG(a6, struct Library* aBase),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
);

ASM(ULONG) SAVEDS AHIsub_Start(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
);

ASM(void) SAVEDS AHIsub_Update(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
);

ASM(void) SAVEDS AHIsub_Stop(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
);

/* Acceleration functions */

ASM(ULONG) SAVEDS AHIsub_SetVol(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aChannel),
  REG(d1, Fixed aVolume),
  REG(d2, sposition aPan),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl),
  REG(d3, ULONG aFlags)
);

ASM(ULONG) SAVEDS AHIsub_SetFreq(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aChannel),
  REG(d1, ULONG aFreq),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl),
  REG(d2, ULONG aFlags)
);

ASM(ULONG) SAVEDS AHIsub_SetSound(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aChannel),
  REG(d1, UWORD aSound),
  REG(d2, ULONG aOffset),
  REG(d3, LONG aLength),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl),
  REG(d4, ULONG aFlags)
);

ASM(ULONG) SAVEDS AHIsub_SetEffect(
  REG(a6, struct Library* aBase),
  REG(a0, APTR aEffect),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
);

ASM(ULONG) SAVEDS AHIsub_LoadSound(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aSound),
  REG(d1, ULONG aType),
  REG(a0, APTR aInfo),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
);

ASM(ULONG) SAVEDS AHIsub_UnloadSound(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aSound),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
);

/* Query functions */

ASM(LONG) SAVEDS AHIsub_GetAttr(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aAttribute),
  REG(d1, LONG aArgument),
  REG(d2, LONG aDefault),
  REG(a1, struct TagItem* aTagList),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
);

/* Mixer functions */

ASM(LONG) SAVEDS AHIsub_HardwareControl(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aAttribute),
  REG(d1, LONG aArgument),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
);

#endif /* SDI_AHI4_PROTOS_H */
