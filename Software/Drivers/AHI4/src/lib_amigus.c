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

#include <intuition/intuitionbase.h>
#include <libraries/expansionbase.h>
#include <proto/amigus.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>

#include "amigus_ahi_sub.h"
#include "debug.h"
#include "errors.h"
#include "library.h"
#include "support.h"

#ifdef BASE_GLOBAL

struct ExecBase          * SysBase           = 0;
struct DosLibrary        * DOSBase           = 0;
struct IntuitionBase     * IntuitionBase     = 0;
struct Library           * UtilityBase       = 0;
struct Library           * ExpansionBase     = 0;
struct Library           * AmiGUS_Base       = 0;
struct Device            * TimerBase         = 0;
struct AmiGUS_AHI_Base   * AmiGUS_AHI_Base   = 0;

#endif

/******************************************************************************
 * Library skeleton required library hooks - public function definitions.
 *****************************************************************************/

LONG CustomLibInit( LIBRARY_TYPE * base, struct ExecBase * sysBase ) {

  struct AmiGUS_AHI_Base * amiGUSBase = (struct AmiGUS_AHI_Base *)base;
  LONG error;

  /* Prevent use of customized library versions on CPUs not targetted. */
#ifdef _M68060
  if( !(sysBase->AttnFlags & AFF_68060) ) {

    return EWrongDriverCPUVersion;
  }
#elif defined (_M68040)
  if( !(sysBase->AttnFlags & AFF_68040) ) {

    return EWrongDriverCPUVersion;
  }
#elif defined (_M68030)
  if( !(sysBase->AttnFlags & AFF_68030) ) {

    return EWrongDriverCPUVersion;
  }
#elif defined (_M68020)
  if( !(sysBase->AttnFlags & AFF_68020) ) {

    return EWrongDriverCPUVersion;
  }
#endif

  amiGUSBase->agb_SysBase = sysBase;

#ifdef BASE_GLOBAL
  SysBase = sysBase;
#endif

  amiGUSBase->agb_LogFile = NULL;
  amiGUSBase->agb_LogMem = NULL;

  amiGUSBase->agb_DOSBase =
    (struct DosLibrary *) OpenLibrary("dos.library", 37);
  if( !(amiGUSBase->agb_DOSBase) ) {

    return EOpenDosBase;
  }
  amiGUSBase->agb_IntuitionBase =
    (struct IntuitionBase *) OpenLibrary("intuition.library", 37);
  if( !(amiGUSBase->agb_IntuitionBase) ) {

    return EOpenIntuitionBase;
  }
  amiGUSBase->agb_UtilityBase = 
    (struct Library *) OpenLibrary("utility.library", 37);
  if( !(amiGUSBase->agb_UtilityBase) ) {

    return EOpenUtilityBase;
  }
  amiGUSBase->agb_ExpansionBase =
    (struct Library *) OpenLibrary("expansion.library", 37);
  if( !(amiGUSBase->agb_ExpansionBase) ) {

    return EOpenExpansionBase;
  }
  base->agb_AmiGUS_Base =
    ( struct Library * ) OpenLibrary( "amigus.library", 1 );
  if ( !( base->agb_AmiGUS_Base )) {

    return EOpenAmiGusBase;
  }
  amiGUSBase->agb_TimerRequest = AllocMem( sizeof(struct IORequest),
                                           MEMF_ANY | MEMF_CLEAR );
  if( !(amiGUSBase->agb_TimerRequest) ) {

    return EAllocateTimerRequest;
  }
  error = OpenDevice("timer.device", 0, amiGUSBase->agb_TimerRequest, 0);
  if ( error ) {

    return EOpenTimerDevice;
  }
  amiGUSBase->agb_TimerBase = amiGUSBase->agb_TimerRequest->io_Device;

#ifdef BASE_GLOBAL
  DOSBase         = amiGUSBase->agb_DOSBase;
  IntuitionBase   = amiGUSBase->agb_IntuitionBase;
  UtilityBase     = amiGUSBase->agb_UtilityBase;
  ExpansionBase   = amiGUSBase->agb_ExpansionBase;
  AmiGUS_Base     = base->agb_AmiGUS_Base;
  TimerBase       = amiGUSBase->agb_TimerBase;
  AmiGUS_AHI_Base = amiGUSBase;
#endif

  LOG_D(( "D: AmiGUS base ready @ 0x%08lx\n", amiGUSBase ));
  amiGUSBase->agb_AmiGUS = AmiGUS_FindCard( amiGUSBase->agb_AmiGUS );
  if ( !( amiGUSBase->agb_AmiGUS )) {

    error = EAmiGUSNotFound;

  } else if ( AMIGUS_AHI_FIRMWARE_MINIMUM 
    > amiGUSBase->agb_AmiGUS->agus_FirmwareRev ) {

    error = EAmiGUSFirmwareOutdated;

  } else {

    error = AmiGUS_ReserveCard( amiGUSBase->agb_AmiGUS,
                                AMIGUS_FLAG_PCM,
                                amiGUSBase );
  }
  if ( error ) {

    DisplayError( error );

  } else {

    amiGUSBase->agb_CardBase = amiGUSBase->agb_AmiGUS->agus_PcmBase;
    amiGUSBase->agb_UsageCounter = 0;
  }
  LOG_I(( "I: %s\n", LIBRARY_IDSTRING ));
  return ENoError;
}

VOID CustomLibClose( LIBRARY_TYPE * base ) {

  struct AmiGUS_AHI_Base * amiGUSBase = (struct AmiGUS_AHI_Base *)base;

  AmiGUS_FreeCard( amiGUSBase->agb_AmiGUS,
                   AMIGUS_FLAG_PCM,
                   amiGUSBase );
#ifndef BASE_GLOBAL
  struct ExecBase *SysBase = AmiGUS_AHI_Base->agb_SysBase;
#endif

  if( amiGUSBase->agb_TimerBase ) {

    CloseDevice( amiGUSBase->agb_TimerRequest );
  }
  if( amiGUSBase->agb_TimerRequest ) {

    FreeMem( amiGUSBase->agb_TimerRequest, sizeof(struct IORequest) );
  }  
  if ( amiGUSBase->agb_LogFile ) {

    Close( amiGUSBase->agb_LogFile );
  }
  /*
  Remember: memory cannot be overwritten if we do not return it. :)
  So... we leak it here... 
  if ( amiGUSBase->agb_LogMem ) {

    FreeMem( amiGUSBase->agb_LogMem, ... );
  }    
  */
  if ( base->agb_AmiGUS_Base ) {

    CloseLibrary(( struct Library * ) base->agb_AmiGUS_Base );
  }
  if( amiGUSBase->agb_ExpansionBase ) {

    CloseLibrary( (struct Library *) amiGUSBase->agb_ExpansionBase );
  }
  if( amiGUSBase->agb_UtilityBase ) {

    CloseLibrary( (struct Library *) amiGUSBase->agb_UtilityBase );
  }
  if( amiGUSBase->agb_IntuitionBase ) {

    CloseLibrary( (struct Library *) amiGUSBase->agb_IntuitionBase );
  }
  if( amiGUSBase->agb_DOSBase ) {

    CloseLibrary( (struct Library *) amiGUSBase->agb_DOSBase );
  }
}
