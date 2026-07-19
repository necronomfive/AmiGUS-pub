/*
 * This file is part of the amigus.library.
 *
 * amigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * amigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with amigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_H
#define AMIGUS_H

#include <amigus/SDI_compiler.h>
#include <exec/types.h>

/**
 * Enumerates all known AmiGUS derivatives with their own ID.
 */
enum AmiGUS_TypeIds {

  AmiGUS_Zorro2 = 0x7000,   // Original Zorro2 card, avoiding Zero collisions
  AmiGUS_mini               // PCMCIA card
};

/**
 * Flags defining all functional blocks / parts of an AmiGUS card.
 */
#define AMIGUS_FLAG_NONE        0x0000   //< No part ;)
#define AMIGUS_FLAG_PCM         0x0001   //< PCM like main part, incl. mixer
#define AMIGUS_FLAG_WAVETABLE   0x0002   //< Wavetable part
#define AMIGUS_FLAG_CODEC       0x0004   //< Codec part, for MP3, FLAC, ...

/**
 * amigus.library error codes as returned by library interface functions.
 */
#define AMIGUS_IN_USE_START     0x0100
enum AmiGUS_Errors {

  AmiGUS_NoError                = 0,
  AmiGUS_InUse                  = AMIGUS_IN_USE_START, // 0x0100
  AmiGUS_PcmInUse               = AMIGUS_IN_USE_START                                             | AMIGUS_FLAG_PCM,
  AmiGUS_WavetableInUse         = AMIGUS_IN_USE_START                     | AMIGUS_FLAG_WAVETABLE                  ,
  AmiGUS_PcmWavetableInUse      = AMIGUS_IN_USE_START                     | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_PCM,
  AmiGUS_CodecInUse             = AMIGUS_IN_USE_START | AMIGUS_FLAG_CODEC                                          ,
  AmiGUS_PcmCodecInUse          = AMIGUS_IN_USE_START | AMIGUS_FLAG_CODEC                         | AMIGUS_FLAG_PCM,
  AmiGUS_WavetableCodecInUse    = AMIGUS_IN_USE_START | AMIGUS_FLAG_CODEC | AMIGUS_FLAG_WAVETABLE                  ,
  AmiGUS_PcmWavetableCodecInUse = AMIGUS_IN_USE_START | AMIGUS_FLAG_CODEC | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_PCM,
  AmiGUS_NotYours               = 0x0200,
  AmiGUS_DetectError            = 0x0401,
  AmiGUS_InterruptInstallFailed = 0x0402,
  AmiGUS_InterruptRemoveFailed  = 0x0403,
  AmiGUS_NotFound               = 0x0404,
  AmiGUS_NotImplemented         = 0x0500
};

/**
 * AmiGUS card description as returned by amigus.library/AmiGUS_FindCard().
 *
 * No need to free it, ownership stays with amigus.library.
 * Consider ALL fields read-only, please.
 */
struct AmiGUS {

  APTR      agus_PcmBase;       //> Base address of the PCM part of the card.
  APTR      agus_WavetableBase; //> Base address of the Wavetable part.
  APTR      agus_CodecBase;     //> Base address of the codec part.

  union {
    ULONG     idLongs[ 2 ];     //> Hardware ID of the card's FPGA - in ULONGs.
    UBYTE     idBytes[ 8 ];     //> Hardware ID of the card's FPGA - in UBYTEs.
  } agus_FpgaId;
  
  ULONG     agus_HardwareRev;   //> Hardware revision of the card.
  ULONG     agus_FirmwareRev;   //> Firmware revision of the card.

  STRPTR    agus_TypeName;      //> Human readable card type string.
  UWORD     agus_TypeId;        //> Card type from enum AmiGUS_TypeIds.

  UWORD     agus_Year;          //> Firmware date, year portion.
  UBYTE     agus_Month;         //> Firmware date, month portion.
  UBYTE     agus_Day;           //> Firmware date, day portion.
  UBYTE     agus_Hour;          //> Firmware date, hour portion.
  UBYTE     agus_Minute;        //> Firmware date, minute portion.
};

/**
 * Woah... what is that?
 *
 * The interrupt handler function's signature as a typedef.
 * Yes, amigus.library/AmiGUS_InstallInterrupt() expects user code delivering a
 * function pointer matching exactly this type! But what is it?
 * - A pointer to a function in register call style,
 * - the function accepts only 1 parameter in d1,
 * - the parameter echos the APTR data parameter delivered 
 *   when the interrupt handler was installed,
 * - the function returns LONG in d0,
 *   0 if the interrupt was not handled,
 *   1 if the interrupt was handled.
 */
typedef ASM( LONG ) ( * AmiGUS_Interrupt )( REG( d1, APTR data ));

#endif /* AMIGUS_H */
