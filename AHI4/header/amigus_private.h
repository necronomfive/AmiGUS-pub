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
#include "library.h"
#include "SDI_AHI4_protos.h"

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
  ULONG                         agb_State;         /* 0 stopped, 1 playing   */

  LONG /* added to desired */(ASM(*) agb_CopyFunction )( REG(d0, ULONG *), REG(a0, ULONG *) );

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

extern const STRPTR AmiGUSOutputs[ AMIGUS_AHI_NUM_OUTPUTS ];
extern const STRPTR AmiGUSInputs[ AMIGUS_AHI_NUM_INPUTS ];

/******************************************************************************
 * Library accessible function
 *****************************************************************************/

LONG FindAmiGUS( struct AmiGUSBasePrivate *amiGUSBase );

LONG FindSampleRateIdForValue( LONG sampleRate );
LONG FindSampleRateValueForId( LONG id );

VOID initAmiGUS( VOID );
VOID stopAmiGUS( VOID );

ASM(LONG) SAVEDS INTERRUPT handleInterrupt(
  REG(a1, struct AmiGUSBasePrivate * amiGUSBase)
);

UWORD ReadReg16( APTR amiGUS, ULONG offset );
VOID WriteReg16( APTR amiGUS, ULONG offset, UWORD value );
ULONG ReadReg32( APTR amiGUS, ULONG offset );
VOID WriteReg32( APTR amiGUS, ULONG offset, ULONG value );

BOOL CreatePlaybackBuffers( VOID );
VOID DestroyPlaybackBuffers( VOID );

BOOL CreateInterruptHandler( VOID );
VOID DestroyInterruptHandler( VOID );

BOOL CreateWorkerProcess( VOID );
VOID DestroyWorkerProcess( VOID );

#endif /* AMIGUS_PRIVATE_H */
