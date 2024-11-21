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

/*
#pragma dontwarn 61
#include <proto/ahi_sub.h>
#pragma popwarn
*/

#include <dos/dos.h>
#include <exec/libraries.h>
#include <libraries/expansionbase.h>
#include <utility/hooks.h>
#include <utility/utility.h>
#include <SDI_compiler.h>

#include "SDI_AHI4_protos.h"
#include "amigus_public.h"

#define VERSION   4
#define REVISION  1
#define DATETXT	  __AMIGADATE__
#define VERSTXT	  "4.1"

#ifdef _M68060
  #define ADDTXT	" 060"
#elif defined(_M68040)
  #define ADDTXT	" 040"
#elif defined(_M68030)
  #define ADDTXT	" 030"
#elif defined(_M68020)
  #define ADDTXT	" 020"
#elif defined(__MORPHOS__)
  #define ADDTXT	" MorphOS"
#else
  #define ADDTXT	""
#endif

#define IDSTRING LIBNAME " " VERSTXT " " DATETXT ADDTXT "\r\n"
/************************************************************************
*                    							*
*    SegList pointer definition						*
*                    							*
************************************************************************/

#if defined(_AROS)
  typedef struct SegList * SEGLISTPTR;
#elif defined(__VBCC__)
  typedef APTR SEGLISTPTR;
#else
  typedef BPTR SEGLISTPTR;
#endif

/************************************************************************
*                    							*
*    library base structure						*
*                    							*
************************************************************************/

/* This is the private structure. The official one does not contain all
the private fields! */
struct AmiGUSBasePrivate {
  /* Library base stuff */
  struct Library                agb_LibNode;
  UWORD                         agb_Unused0;    /* better alignment */

  struct ExecBase             * agb_SysBase;
  struct DosLibrary           * agb_DOSBase;
  struct IntuitionBase        * agb_IntuitionBase;
  struct Library              * agb_UtilityBase;
  struct Library              * agb_ExpansionBase;
  SEGLISTPTR	                agb_SegList;

  struct Device               * TimerBase;
  struct IORequest            * TimerRequest;
  /* AmiGUS specific member variables */
  APTR                          agb_CardBase;
  struct Interrupt            * agb_Interrupt;
  struct Process              * agb_MainProcess;
  struct Process              * agb_WorkerProcess;
  BYTE                          agb_MainSignal;
  BYTE                          agb_WorkerWorkSignal;
  BYTE                          agb_WorkerStopSignal;
  BYTE                          agb_UsageCounter;    
  /* Only 1 AmiGUS supported per machine currently, sorry */

  /* Mixing double-buffers to be copied to FIFO alternatingly */
  ULONG                       * agb_Buffer[2];      /* Fully LONG aligned!   */
  ULONG                         agb_BufferIndex[2]; /* Next LONG index each  */
  ULONG                         agb_BufferMax[2];   /* LONGs watermark each  */
  ULONG                         agb_BufferSize;     /* LONGs allocated each  */
  ULONG                         agb_currentBuffer;  /* Current playing buf.  */

  ULONG                         agb_watermark;      /* Counting in WORDs!    */
  LONG                          agb_WorkerReady;
  
  struct AHIAudioCtrlDrv      * agb_AudioCtrl;
  ULONG                         agb_State; // 0 stopped, 1 playing

  BPTR                          agb_LogFile;       /* Debug log file handle */
  APTR                          agb_LogMem;        /* Debug log memory blob */
};

#if defined(BASE_GLOBAL)
  extern struct ExecBase          * SysBase;
  extern struct DosLibrary        * DOSBase;
  extern struct IntuitionBase     * IntuitionBase;
  extern struct Library           * UtilityBase;
  extern struct Library           * ExpansionBase;
  extern struct AmiGUSBasePrivate * AmiGUSBase;

extern struct Device               * TimerBase;
#elif defined(BASE_REDEFINE)
  #define SysBase               amiGUSBase->agb_SysBase
  #define DOSBase               amiGUSBase->agb_DOSBase
  #define IntuitionBase         amiGUSBase->agb_IntuitionBase
  #define UtilityBase           amiGUSBase->agb_UtilityBase
  #define ExpansionBase         amiGUSBase->agb_ExpansionBase
  #define AmiGUSBase            (amiGUSBase)
#define TimerBase amiGUSBase->TimerBase
#endif

/************************************************************************
*                    							*
*    library public function						*
*                    							*
************************************************************************/

#define LIBRARY_FUNCTIONS ( APTR ) AHIsub_AllocAudio, \
                          ( APTR ) AHIsub_FreeAudio, \
                          ( APTR ) AHIsub_Disable, \
                          ( APTR ) AHIsub_Enable, \
                          ( APTR ) AHIsub_Start, \
                          ( APTR ) AHIsub_Update, \
                          ( APTR ) AHIsub_Stop, \
                          ( APTR ) AHIsub_SetVol, \
                          ( APTR ) AHIsub_SetFreq, \
                          ( APTR ) AHIsub_SetSound, \
                          ( APTR ) AHIsub_SetEffect, \
                          ( APTR ) AHIsub_LoadSound, \
                          ( APTR ) AHIsub_UnloadSound, \
                          ( APTR ) AHIsub_GetAttr, \
                          ( APTR ) AHIsub_HardwareControl


/************************************************************************
*                    							*
*    library accessable function					*
*                    							*
************************************************************************/

LONG FindAmiGUS(struct AmiGUSBasePrivate *amiGUSBase);
LONG FindNearestFrequencyIndex(LONG aFrequency);
LONG FindNearestFrequency(LONG aFrequency);
void initAmiGUS(void);
void stopAmiGUS(void);
ASM(LONG) SAVEDS INTERRUPT handleInterrupt(
  REG(a1, struct AmiGUSBasePrivate * amiGUSBase)
);

UWORD ReadReg16(APTR amiGUS, ULONG offset);
void WriteReg16(APTR amiGUS, ULONG offset, UWORD value);
ULONG ReadReg32(APTR amiGUS, ULONG offset);
void WriteReg32(APTR amiGUS, ULONG offset, ULONG value);

BOOL CreatePlaybackBuffers(VOID);
VOID DestroyPlaybackBuffers(VOID);

BOOL CreateInterruptHandler(VOID);
VOID DestroyInterruptHandler(VOID);

BOOL CreateWorkerProcess(VOID);
VOID DestroyWorkerProcess(VOID);

extern const LONG Frequencies[];

/*
ASM(LONG) LIBex_TestRequest(
    REG(a0, UBYTE *title), 
    REG(a1, UBYTE *body),
    REG(a2, UBYTE *gadgets), 
    REG(a6, struct FlatEarthBasePrivate * flatEarthBase));
*/

#endif /* AMIGUS_PRIVATE_H */
