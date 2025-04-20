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
struct AmiGUS_MHI        * AmiGUS_MHI_Base   = 0;

#endif

/* Closes all the libraries opened by LibInit() */
VOID CustomLibClose( LIBRARY_TYPE * base ) {

#ifndef BASE_GLOBAL
  struct ExecBase *SysBase = base->agb_SysBase;
#endif

  if ( base->agb_LogFile ) {

    Close( base->agb_LogFile );
  }
  /*
  Remember: memory cannot be overwritten if we do not return it. :)
  So... we leak it here... 
  if ( base->agb_LogMem ) {

    FreeMem( base->agb_LogMem, ... );
  }    
  */
  if ( base->agb_DOSBase ) {

    CloseLibrary(( struct Library *) base->agb_DOSBase );
  }
  if ( base->agb_IntuitionBase ) {

    CloseLibrary(( struct Library * ) base->agb_IntuitionBase );
  }
  if ( base->agb_ExpansionBase ) {

    CloseLibrary(( struct Library * ) base->agb_ExpansionBase );
  }
}

LONG CustomLibInit( LIBRARY_TYPE * base, struct ExecBase * sysBase ) {

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

  base->agb_SysBase = sysBase;

#ifdef BASE_GLOBAL
  SysBase = sysBase;
#endif

  base->agb_LogFile = NULL;
  base->agb_LogMem = NULL;

  base->agb_DOSBase =
    ( struct DosLibrary * ) OpenLibrary( "dos.library", 34 );
  if ( !( base->agb_DOSBase )) {

    return EOpenDosBase;
  }
  base->agb_IntuitionBase =
    ( struct IntuitionBase * ) OpenLibrary( "intuition.library", 34 );
  if ( !( base->agb_IntuitionBase )) {

    return EOpenIntuitionBase;
  }
  base->agb_ExpansionBase =
    ( struct Library * ) OpenLibrary( "expansion.library", 34 );
  if ( !( base->agb_ExpansionBase )) {

    return EOpenExpansionBase;
  }

#ifdef BASE_GLOBAL
  DOSBase         = base->agb_DOSBase;
  IntuitionBase   = base->agb_IntuitionBase;
  ExpansionBase   = base->agb_ExpansionBase;
  AmiGUS_MHI_Base = base;
#endif

  NEW_LIST( &( base->agb_Clients ));

  LOG_D(("D: AmiGUS base ready @ 0x%08lx\n", base));
  return ENoError;
}
