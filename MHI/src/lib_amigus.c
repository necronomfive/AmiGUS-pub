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

#include <intuition/intuitionbase.h>
#include <libraries/expansionbase.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>

#include "amigus_codec.h"
#include "amigus_mhi.h"
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
struct Device            * TimerBase         = 0;
struct AmiGUSBase        * AmiGUSBase        = 0;

#endif

/* Closes all the libraries opened by LibInit() */
VOID CustomLibClose( struct BaseLibrary * base ) {

  struct AmiGUSBase * amiGUSBase = (struct AmiGUSBase *)base;

#ifndef BASE_GLOBAL
  struct ExecBase *SysBase = AmiGUSBase->agb_SysBase;
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

  if( amiGUSBase->agb_DOSBase ) {

    CloseLibrary( (struct Library *) amiGUSBase->agb_DOSBase );
  }
  if( amiGUSBase->agb_IntuitionBase ) {

    CloseLibrary( (struct Library *) amiGUSBase->agb_IntuitionBase );
  }
  if( amiGUSBase->agb_UtilityBase ) {

    CloseLibrary( (struct Library *) amiGUSBase->agb_UtilityBase );
  }
  if( amiGUSBase->agb_ExpansionBase ) {

    CloseLibrary( (struct Library *) amiGUSBase->agb_ExpansionBase );
  }
}

LONG CustomLibInit( struct BaseLibrary * base, struct ExecBase * sysBase ) {

  struct AmiGUSBase * amiGUSBase = (struct AmiGUSBase *)base;
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
  DOSBase       = amiGUSBase->agb_DOSBase;
  IntuitionBase = amiGUSBase->agb_IntuitionBase;
  UtilityBase   = amiGUSBase->agb_UtilityBase;
  ExpansionBase = amiGUSBase->agb_ExpansionBase;
  TimerBase     = amiGUSBase->agb_TimerBase;
  AmiGUSBase    = amiGUSBase;
#endif

  LOG_D(("D: AmiGUS base ready @ 0x%08lx\n", amiGUSBase));
  error = FindAmiGusCodec( amiGUSBase );
  if ( error ) {

    DisplayError( error );
  }
  return ENoError;
}
