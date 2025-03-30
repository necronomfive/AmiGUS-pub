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

#ifndef LIBRARY_H
#define LIBRARY_H

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <libraries/dos.h>

/******************************************************************************
 * Define your library's public functions here,
 * will be used in library.c.
 *****************************************************************************/

#define LIBRARY_FUNCTIONS ( APTR ) MHIAllocDecoder, \
                          ( APTR ) MHIFreeDecoder, \
                          ( APTR ) MHIQueueBuffer, \
                          ( APTR ) MHIGetEmpty, \
                          ( APTR ) MHIGetStatus, \
                          ( APTR ) MHIPlay, \
                          ( APTR ) MHIStop, \
                          ( APTR ) MHIPause, \
                          ( APTR ) MHIQuery, \
                          ( APTR ) MHISetParam

/******************************************************************************
 * Define your library's properties here,
 * will be used in library.c.
 *****************************************************************************/

#define STR_VALUE(x)      #x
#define STR(x)            STR_VALUE(x)

#define LIBRARY_NAME      LIB_FILE
#define LIBRARY_VERSI0N   LIB_VERSION
#define LIBRARY_REVISION  LIB_REVISION
#define LIBRARY_IDSTRING  STR( LIB_FILE )" "                         \
                          STR( LIB_VERSION )".00"STR( LIB_REVISION ) \
                          " "LIB_DATE" "STR( LIB_CPU )" "            \
                          STR( LIB_COMPILER )" "STR( LIB_HOST )

/******************************************************************************
 * SegList pointer definition
 *****************************************************************************/

#if defined(_AROS)
  typedef struct SegList * SEGLISTPTR;
#elif defined(__VBCC__)
  typedef APTR SEGLISTPTR;
#else
  typedef BPTR SEGLISTPTR;
#endif

/******************************************************************************
 * Library base structure - ALWAYS make it the first element (!!!)
 * in whatever your custom library's structure looks like.
 *****************************************************************************/
struct BaseLibrary {

  struct Library                LibNode;
  UWORD                         Unused0;                 /* better alignment */
  SEGLISTPTR                    SegList;
  struct SignalSemaphore        LockSemaphore;
  struct ExecBase             * SysBase;
};

/******************************************************************************
 * Define your library's base type here, will be used in library.c.
 *****************************************************************************/
#define LIBRARY_TYPE      struct AmiGUS_MHI

LIBRARY_TYPE;

/******************************************************************************
 * Your library's own base structure shall have its own include,
 * maybe together with your library specific functions.
 * Include it here!
 *****************************************************************************/
#include "amigus_mhi.h"

/******************************************************************************
 * Now go ahead and implement these functions in your library adapter code!
 *****************************************************************************/
LONG CustomLibInit( LIBRARY_TYPE * base, struct ExecBase * sysBase );
VOID CustomLibClose( LIBRARY_TYPE * base );

#endif /* LIBRARY_H */
