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


#include <exec/libraries.h>

#include "SDI_AHI4_protos.h"

#ifndef LOG_D
#define LOG_D(X)
#endif

/* Acceleration functions */

ASM(ULONG) SAVEDS AHIsub_SetEffect(
  REG(a6, struct Library* aBase),
  REG(a0, APTR aEffect),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
  LOG_D(("AHIsub_SetEffect\n"));
  return AHIS_UNKNOWN;
}

ASM(ULONG) SAVEDS AHIsub_SetFreq(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aChannel),
  REG(d1, ULONG aFreq),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl),
  REG(d2, ULONG aFlags)
) {
  LOG_D(("AHIsub_SetFreq\n"));
  return AHIS_UNKNOWN;
}

ASM(ULONG) SAVEDS AHIsub_SetSound(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aChannel),
  REG(d1, UWORD aSound),
  REG(d2, ULONG aOffset),
  REG(d3, LONG aLength),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl),
  REG(d4, ULONG aFlags)
) {
  LOG_D(("AHIsub_SetSound\n"));
  return AHIS_UNKNOWN;
}

ASM(ULONG) SAVEDS AHIsub_SetVol(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aChannel),
  REG(d1, Fixed aVolume),
  REG(d2, sposition aPan),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl),
  REG(d3, ULONG aFlags)
) {
  LOG_D(("AHIsub_SetVol\n"));
  return AHIS_UNKNOWN;
}

ASM(ULONG) SAVEDS AHIsub_LoadSound(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aSound),
  REG(d1, ULONG aType),
  REG(a0, APTR aInfo),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
  LOG_D(("AHIsub_LoadSound\n"));
  return AHIS_UNKNOWN;
}

ASM(ULONG) SAVEDS AHIsub_UnloadSound(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aSound),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
  LOG_D(("AHIsub_UnloadSound\n"));
  return AHIS_UNKNOWN;
}
