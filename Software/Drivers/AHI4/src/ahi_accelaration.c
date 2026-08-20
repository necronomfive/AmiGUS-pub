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


#include <exec/libraries.h>

#include "ahi_sub_protos.h"

#ifndef LOG_D
#define LOG_D(X)
#endif

/* Acceleration functions */

ULONG __ASM__ __SAVE_DS__ AHIsub_SetEffect(
  __REG__( a0, APTR aEffect ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
) {

  LOG_D(("AHIsub_SetEffect\n"));
  return AHIS_UNKNOWN;
}

ULONG __ASM__ __SAVE_DS__ AHIsub_SetFreq(
  __REG__( d0, UWORD aChannel ),
  __REG__( d1, ULONG aFreq ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl ),
  __REG__( d2, ULONG aFlags )
) {

  LOG_D(("AHIsub_SetFreq\n"));
  return AHIS_UNKNOWN;
}

ULONG __ASM__ __SAVE_DS__ AHIsub_SetSound(
  __REG__(d0, UWORD aChannel),
  __REG__(d1, UWORD aSound),
  __REG__(d2, ULONG aOffset),
  __REG__(d3, LONG aLength),
  __REG__(a2, struct AHIAudioCtrlDrv * aAudioCtrl),
  __REG__(d4, ULONG aFlags)
) {

  LOG_D(("AHIsub_SetSound\n"));
  return AHIS_UNKNOWN;
}

ULONG __ASM__ __SAVE_DS__ AHIsub_SetVol(
  __REG__( d0, UWORD aChannel ),
  __REG__( d1, Fixed aVolume ),
  __REG__( d2, sposition aPan ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl ),
  __REG__( d3, ULONG aFlags )
) {

  LOG_D(("AHIsub_SetVol\n"));
  return AHIS_UNKNOWN;
}

ULONG __ASM__ __SAVE_DS__ AHIsub_LoadSound(
  __REG__( d0, UWORD aSound ),
  __REG__( d1, ULONG aType ),
  __REG__( a0, APTR aInfo ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
) {

  LOG_D(("AHIsub_LoadSound\n"));
  return AHIS_UNKNOWN;
}

ULONG __ASM__ __SAVE_DS__ AHIsub_UnloadSound(
  __REG__( d0, UWORD aSound ),
  __REG__( a2, struct AHIAudioCtrlDrv * aAudioCtrl )
) {

  LOG_D(("AHIsub_UnloadSound\n"));
  return AHIS_UNKNOWN;
}
