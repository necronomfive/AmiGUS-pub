/*
 * This file is part of the mhiAmiGUS.library.
 *
 * mhiAmiGUS.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_MHI_H
#define AMIGUS_MHI_H

/*
 * MHI driver library header.
 *
 * To be used only internally - but there in all .c files!
 * If you are using some of the library base addresses from
 * BASE_REDEFINE directly, e.g. in libinit.c,
 * do a #define NO_BASE_REDEFINE before including this file.
 */

/* Activate / De-activate this define to toggle lib base mode! */
#define BASE_GLOBAL /**/

#ifndef BASE_GLOBAL 
#ifndef NO_BASE_REDEFINE
/* 
 * either this is active for everything except libinit.c
 * or BASE_GLOBAL is active everywhere
 */
#define BASE_REDEFINE
#endif
#endif

#include "library.h"

#define AMIGUS_MHI_AUTHOR           "Christoph `Chritoph` Fassbach"
#define AMIGUS_MHI_COPYRIGHT        "(c) 2025 Christoph Fassbach / LGPL3"
#define AMIGUS_MHI_ANNOTATION       "Thanks to: Oliver Achten (AmiGUS), " \
                                    "Frank Wille (vbcc), "                \
                                    "Thomas Wenzel et al. (MHI)"
#define AMIGUS_MHI_DECODER          "AmiGUS VS1063a codec"
#define AMIGUS_MHI_VERSION          LIBRARY_IDSTRING

#define AMIGUS_MHI_FIRMWARE_MINIMUM ( ( 2025 << 20 ) /* year   */ \
                                    + (    3 << 16 ) /* month  */ \
                                    + (   24 << 11 ) /* day    */ \
                                    + (   21 <<  6 ) /* hour   */ \
                                    + (   38 <<  0 ) /* minute */ )

#define AMIGUS_MEM_LOG_BORDERS      "********************************"

/******************************************************************************
 * Library base structure components
 *****************************************************************************/

struct AmiGUS_MHI_Buffer {

  struct MinNode                agmb_Node;

  ULONG                       * agmb_Buffer;
  ULONG                         agmb_BufferIndex;
  ULONG                         agmb_BufferMax;
  ULONG                         agmb_BufferExtraBytes; // Only 0-3 - :)
};

struct AmiGUS_MHI_Handle {
  struct Task                 * agch_Task;
  LONG                          agch_Signal;

  struct MinList                agch_Buffers;
  struct AmiGUS_MHI_Buffer    * agch_CurrentBuffer;

  UBYTE                         agch_MHI_Equalizer[ 11 ]; /* 10 band, 1 gain */
  UBYTE                         agch_Status;
};

/******************************************************************************
 * Library base structure
 *****************************************************************************/

/* This is the private structure. The official one does not contain all
the private fields! */
struct AmiGUS_MHI {
  /* Library base stuff */
  struct BaseLibrary            agb_BaseLibrary;

  struct ExecBase             * agb_SysBase;
  struct DosLibrary           * agb_DOSBase;
  struct IntuitionBase        * agb_IntuitionBase;
  struct Library              * agb_ExpansionBase;

  /* AmiGUS specific member variables */
  struct ConfigDev            * agb_ConfigDevice;
  APTR                          agb_CardBase;
  struct Interrupt            * agb_Interrupt;

  /* Client info */
  struct AmiGUS_MHI_Handle      agb_ClientHandle;

  /* Only 1 AmiGUS supported per machine currently, sorry */
  BYTE                          agb_UsageCounter;
  UBYTE                         agb_Reserved0;
  UWORD                         agb_Reserved1;

  BPTR                          agb_LogFile;       /* Debug log file handle  */
  APTR                          agb_LogMem;        /* Debug log memory blob  */
};

#if defined(BASE_GLOBAL)
  extern struct AmiGUS_MHI        * AmiGUS_MHI_Base;
  extern struct DosLibrary        * DOSBase;
  extern struct Library           * ExpansionBase;
  extern struct IntuitionBase     * IntuitionBase;
  extern struct ExecBase          * SysBase;
#elif defined(BASE_REDEFINE)
  #define AmiGUS_MHI_Base           (base)
  #define DOSBase                   base->agb_DOSBase
  #define ExpansionBase             base->agb_ExpansionBase
  #define IntuitionBase             base->agb_IntuitionBase
  #define SysBase                   base->agb_SysBase
#endif

/******************************************************************************
 * Library flag definitions
 *****************************************************************************/

#endif /* AMIGUS_MHI_H */
