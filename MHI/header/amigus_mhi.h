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

#include "amigus/amigus.h"

#include "library.h"

/*
 * Query-able facts via MHIQuery function:
 */
#define AMIGUS_MHI_AUTHOR           "Christoph `Chritoph` Fassbach"
#define AMIGUS_MHI_COPYRIGHT        "(c) 2025 Christoph Fassbach / LGPL3"
#define AMIGUS_MHI_ANNOTATION       "Thanks to: Oliver Achten (AmiGUS), " \
                                    "Frank Wille (vbcc), "                \
                                    "Thomas Wenzel et al. (MHI)"
#define AMIGUS_MHI_DECODER          "AmiGUS VS1063a codec"
#define AMIGUS_MHI_VERSION          LIBRARY_IDSTRING

/*
 * Minimum firmware required to use this version of the MHI driver,
 * e.g. the codec part's timer came late in the process to ditch
 * the AmigaOS 2.0/v36 requirement.
 */
#define AMIGUS_MHI_FIRMWARE_MINIMUM ( ( 2025 << 20 ) /* year   */ \
                                    + (    3 << 16 ) /* month  */ \
                                    + (   24 << 11 ) /* day    */ \
                                    + (   21 <<  6 ) /* hour   */ \
                                    + (   38 <<  0 ) /* minute */ )

/*
 * If logging to memory is activated, this is used to mark the start
 * of the log memory. And as 1 pointer to this marker, 1 pointer to the
 * library file name and 1 more pointer to the marker are used, the full
 * start marker is not even in the library, no need to unload the library
 * so even with library in memory it should unique in memory.
 */
#define AMIGUS_MEM_LOG_BORDERS      "********************************"

/******************************************************************************
 * Library base structure components
 *****************************************************************************/

/**
 * Instances of this struct are used to manage the buffers the client
 * wants to feed to the driver.
 *
 * The client can provide any buffer size,
 * but ...
 * a) the AmiGUS Hardware does not allow BYTE access,
 * b) the BYTE access performance is just bad.
 * So per default, the buffer is LONGs for optimized access
 * and we have 0 to 3 additional BYTEs to copy in with performance impact.
 * As clients (HippoPlayer, AmigaAMP, ...) usually define the buffers in
 * kB or kiB, we are kind of safe to assume we only see LONGs as both are
 * dividable by 4. Therefore, 1-3 remaining BYTEs are handled only for the
 * end of the file and padded with the "end fill bytes" not causing any
 * nasty noise when decoded.
 */
struct AmiGUS_MHI_Buffer {

  struct MinNode                agmb_Node;             // Yes, a list node

  ULONG                       * agmb_Buffer;           // Client owned buffer
  ULONG                         agmb_BufferIndex;      // Next index to play
  ULONG                         agmb_BufferMax;        // Buffer size in LONGs
  ULONG                         agmb_BufferExtraBytes; // Extra BYTE count, 0-3
};

/**
 * Instances of this struct are handed back to the clients to hold the state
 * of the driver and AmiGUS hardware.
 *
 * The driver can hand out multiple client handles,
 * as long as these are either not bound to an AmiGUS hardware or
 * there is more hardware around - kind of (untested) multi-card support.
 */
struct AmiGUS_MHI_Handle {

  APTR                          agch_CardBase;      // Codec base address
  struct AmiGUS               * agch_AmiGUS;        // AmiGUS card handle

  struct Task                 * agch_Task;          // Client task and ...
  LONG                          agch_Signal;        // ... signal to notify

  struct MinList                agch_Buffers;       // AmiGUS_MHI_Buffer list
  struct AmiGUS_MHI_Buffer    * agch_CurrentBuffer; // Buffer to play next

  UBYTE                         agch_MHI_Panning;   // Balance applied
  UBYTE                         agch_MHI_Volume;    // Playback volume 
  UWORD                         agch_reserved0;     // Padding LONG alignment
  UBYTE                         agch_MHI_Equalizer[ 11 ]; // 10 band, 1 gain
  UBYTE                         agch_Status;        // MHI status of client
};

/******************************************************************************
 * Library base structure
 *****************************************************************************/

/**
 * Private AmiGUS MHI library base structure.
 *
 * There is no public one, pointers to libraries opened, interrupts,
 * list of client handles, logs. Nothing to play around with.
 */
struct AmiGUS_MHI {
  /* Library base stuff */
  struct BaseLibrary            agb_BaseLibrary;   // Instance of library.h

  struct ExecBase             * agb_SysBase;       // Exec, allocations etc.
  struct DosLibrary           * agb_DOSBase;       // DOS, logs and so on
  struct IntuitionBase        * agb_IntuitionBase; // For error messages
  struct Library              * agb_AmiGUS_Base;   // Finding & handling AmiGUS

  BPTR                          agb_LogFile;       // Debug log file handle
  APTR                          agb_LogMem;        // Debug log memory blob
};

/*
 * All libraries' base pointers used by the MHI driver library.
 * Also used to switch between relying on globals or not.
 */
#if defined(BASE_GLOBAL)
  extern struct AmiGUS_MHI        * AmiGUS_MHI_Base;
  extern struct DosLibrary        * DOSBase;
  extern struct Library           * AmiGUS_Base;
  extern struct IntuitionBase     * IntuitionBase;
  extern struct ExecBase          * SysBase;
#elif defined(BASE_REDEFINE)
  #define AmiGUS_MHI_Base           (base)
  #define DOSBase                   base->agb_DOSBase
  #define AmiGUS_Base               base->agb_AmiGUS_Base
  #define IntuitionBase             base->agb_IntuitionBase
  #define SysBase                   base->agb_SysBase
#endif

#endif /* AMIGUS_MHI_H */
