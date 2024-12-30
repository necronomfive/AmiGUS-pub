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

#ifndef AMIGUS_PRIVATE_H
#define AMIGUS_PRIVATE_H

/* 
* Private library header.
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

#include <dos/dos.h>
#include <libraries/expansionbase.h>
#include <utility/hooks.h>

#include "amigus_public.h"
#include "buffers.h"
#include "library.h"
#include "SDI_AHI4_protos.h"

/******************************************************************************
 * Library base structure components
 *****************************************************************************/

struct AmiGUSPcmPlayback {
  /* Mixing double-buffers to be copied to FIFO alternatingly */  
  ULONG                       * agpp_Buffer[2];      /* Fully LONG aligned!  */
  ULONG                         agpp_BufferIndex[2]; /* Next LONG index each */
  ULONG                         agpp_BufferMax[2];   /* LONGs watermark each */
  ULONG                         agpp_BufferSize;     /* BYTEs allocated each */
  ULONG                         agpp_CurrentBuffer;  /* Current playing buf. */

  CopyFunctionType              agpp_CopyFunction;   /* Magic AHI<->AmiGUS.. */
  ULONG                         agpp_CopyFunctionId; /* ID of CopyFunction   */

  ULONG                         agpp_Watermark;      /* Counting in WORDs!   */
};

/******************************************************************************
 * Library base structure
 *****************************************************************************/

/* This is the private structure. The official one does not contain all
the private fields! */
struct AmiGUSBasePrivate {
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
  UWORD                         agb_HwSampleRateId; /* HW sample rate ID     */
  UWORD                         agb_HwSampleFormat; /* HW sample format ID   */
  UBYTE                         agb_AhiSampleSize;  /* BYTE size of 1 sample */
  UBYTE                         agb_AhiSampleShift; /* Sample <=> Byte shift */
  UBYTE                         agb_CanRecord;      /* Can record? Yes / No  */
  UBYTE                         agb_StateFlags;     /* AmiGUS state as below */

  struct AmiGUSPcmPlayback      agb_Playback;       /* Groups playback vars  */
  
  struct AHIAudioCtrlDrv      * agb_AudioCtrl;

  BPTR                          agb_LogFile;       /* Debug log file handle  */
  APTR                          agb_LogMem;        /* Debug log memory blob  */
};

#if defined(BASE_GLOBAL)
  extern struct AmiGUSBasePrivate * AmiGUSBase;
  extern struct DosLibrary        * DOSBase;
  extern struct Library           * ExpansionBase;
  extern struct IntuitionBase     * IntuitionBase;
  extern struct ExecBase          * SysBase;
  extern struct Device            * TimerBase;
  extern struct Library           * UtilityBase;
#elif defined(BASE_REDEFINE)
  #define AmiGUSBase                (amiGUSBase)
  #define DOSBase                   amiGUSBase->agb_DOSBase
  #define ExpansionBase             amiGUSBase->agb_ExpansionBase
  #define IntuitionBase             amiGUSBase->agb_IntuitionBase
  #define SysBase                   amiGUSBase->agb_SysBase
  #define TimerBase                 amiGUSBase->TimerBase
  #define UtilityBase               amiGUSBase->agb_UtilityBase
#endif

/******************************************************************************
 * Library flag definitions
 *****************************************************************************/

#define AMIGUS_AHI_F_PLAY_STARTED        0x01
#define AMIGUS_AHI_F_PLAY_UNDERRUN       0x02
#define AMIGUS_AHI_F_PLAY_0              0x04
#define AMIGUS_AHI_F_PLAY_1              0x08
#define AMIGUS_AHI_F_REC_STARTED         0x10
#define AMIGUS_AHI_F_REC_0               0x20
#define AMIGUS_AHI_F_REC_1               0x40
#define AMIGUS_AHI_F_REC_2               0x80

#define AMIGUS_AHI_F_PLAY_MASK           0x0F
#define AMIGUS_AHI_F_PLAY_STOP_MASK      0xF0
#define AMIGUS_AHI_F_REC_MASK            0xF0
#define AMIGUS_AHI_F_REC_STOP_MASK       0x0F

#endif /* AMIGUS_PRIVATE_H */
