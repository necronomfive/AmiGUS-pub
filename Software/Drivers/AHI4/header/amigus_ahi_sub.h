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

#ifndef AMIGUS_AHI_SUB_H
#define AMIGUS_AHI_SUB_H

/* 
* AHI driver library header.
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
    either this is active for everything except libinit.c
    or BASE_GLOBAL is active everywhere
*/
#define BASE_REDEFINE
#endif
#endif

#include "copies.h"
#include "library.h"
#include "SDI_ahi_sub_protos.h"

#define AMIGUS_AHI_AUTHOR           "Christoph `Chritoph` Fassbach"
#define AMIGUS_AHI_COPYRIGHT        "(c) 2024 Christoph Fassbach / LGPL3"
#define AMIGUS_AHI_ANNOTATION       "Thanks to: Oliver Achten (AmiGUS), " \
                                    "Frank Wille (vbcc), Martin Blom (AHI)"
#define AMIGUS_AHI_VERSION          LIBRARY_IDSTRING

#define AMIGUS_AHI_FIRMWARE_MINIMUM ( ( 2024 << 20 ) /* year   */ \
                                    + (   12 << 16 ) /* month  */ \
                                    + (    8 << 11 ) /* day    */ \
                                    + (   22 <<  6 ) /* hour   */ \
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

struct AmiGUSPcmPlayback {
  /* Mixing/playback double-buffers to be copied to FIFO alternatingly       */
  ULONG                       * agpp_Buffer[2];      /* Fully LONG aligned!  */
  ULONG                         agpp_BufferIndex[2]; /* Next LONG index each */
  ULONG                         agpp_BufferMax[2];   /* LONGs high mark each */
  ULONG                         agpp_CurrentBuffer;  /* Current playing buf. */

  CopyFunctionType              agpp_CopyFunction;   /* Magic AHI<->AmiGUS.. */

  ULONG                         agpp_Watermark;      /* Counting in WORDs!   */

  ULONG                         agpp_HwSampleSize; /* Size a [stereo] sample */
};

struct AmiGUSPcmRecording {
  /* Recording double-buffers filled from FIFO/emptied by AHI alternatingly  */
  ULONG                       * agpr_Buffer[2];      /* Fully LONG aligned!  */
  ULONG                         agpr_BufferIndex[2]; /* Next LONG index each */
  ULONG                         agpr_BufferMax[2];   /* LONGs high mark each */

  ULONG                         agpr_CurrentBuffer;  /* Current recording b. */

  CopyFunctionType              agpr_CopyFunction;   /* Magic AmiGUS<->AHI.. */
  ULONG                         agpr_CopyInputSize;  /* Function input BYTEs */

  struct AHIRecordMessage       agpr_RecordingMessage;

  ULONG                         agpr_AhiSampleShift; /* Sample <> Byte shift */

  ULONG                         agpr_HwSourceId;          /* Input source ID */
};

/******************************************************************************
 * Library base structure
 *****************************************************************************/

/* This is the private structure. The official one does not contain all
the private fields! */
struct AmiGUS_AHI_Base {
  /* Library base stuff */
  struct BaseLibrary            agb_BaseLibrary;

  struct ExecBase             * agb_SysBase;
  struct DosLibrary           * agb_DOSBase;
  struct IntuitionBase        * agb_IntuitionBase;
  struct Library              * agb_UtilityBase;
  struct Library              * agb_ExpansionBase;

  struct Device               * agb_TimerBase;
  struct IORequest            * agb_TimerRequest;
  /* AmiGUS specific member variables */
  APTR                          agb_CardBase;
  struct Interrupt            * agb_Interrupt;
  struct Process              * agb_MainProcess;
  struct Process              * agb_WorkerProcess;
  LONG                          agb_WorkerReady;
  BYTE                          agb_MainSignal;
  BYTE                          agb_WorkerWorkSignal;
  BYTE                          agb_WorkerStopSignal;
  /* Only 1 AmiGUS supported per machine currently, sorry */
  BYTE                          agb_UsageCounter;    

  /* Driver settings */
  UBYTE                         agb_AhiModeOffset;
  UBYTE                         agb_HwSampleRateId; /* HW sample rate ID     */
  UBYTE                         agb_CanRecord;      /* Can record? Yes / No  */
  UBYTE                         agb_StateFlags;     /* AmiGUS state as below */

  struct AmiGUSPcmPlayback      agb_Playback;       /* Playback vars group   */
  struct AmiGUSPcmRecording     agb_Recording;      /* Recording vars group  */
  
  struct AHIAudioCtrlDrv      * agb_AudioCtrl;

  BPTR                          agb_LogFile;       /* Debug log file handle  */
  APTR                          agb_LogMem;        /* Debug log memory blob  */
};

#if defined(BASE_GLOBAL)
  extern struct AmiGUS_AHI_Base   * AmiGUS_AHI_Base;
  extern struct DosLibrary        * DOSBase;
  extern struct Library           * ExpansionBase;
  extern struct IntuitionBase     * IntuitionBase;
  extern struct ExecBase          * SysBase;
  extern struct Device            * TimerBase;
  extern struct Library           * UtilityBase;
#elif defined(BASE_REDEFINE)
  #define AmiGUS_AHI_Base           (base)
  #define DOSBase                   base->agb_DOSBase
  #define ExpansionBase             base->agb_ExpansionBase
  #define IntuitionBase             base->agb_IntuitionBase
  #define SysBase                   base->agb_SysBase
  #define TimerBase                 base->TimerBase
  #define UtilityBase               base->agb_UtilityBase
#endif

/******************************************************************************
 * Library flag definitions
 *****************************************************************************/

#define AMIGUS_AHI_F_PLAY_STARTED        0x01
#define AMIGUS_AHI_F_PLAY_UNDERRUN       0x02
#define AMIGUS_AHI_F_PLAY_0              0x04
#define AMIGUS_AHI_F_PLAY_1              0x08
#define AMIGUS_AHI_F_REC_STARTED         0x10
#define AMIGUS_AHI_F_REC_OVERFLOW        0x20
#define AMIGUS_AHI_F_REC_1               0x40
#define AMIGUS_AHI_F_REC_2               0x80

#define AMIGUS_AHI_F_PLAY_MASK           0x0F
#define AMIGUS_AHI_F_PLAY_STOP_MASK      0xF0
#define AMIGUS_AHI_F_REC_MASK            0xF0
#define AMIGUS_AHI_F_REC_STOP_MASK       0x0F

#endif /* AMIGUS_AHI_SUB_H */
