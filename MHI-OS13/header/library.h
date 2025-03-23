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

#include <exec/libraries.h>
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
/*
#define LIBRARY_NAME      GSTR( LIB_FILE )
#define LIBRARY_VERSION   1
#define LIBRARY_REVISION  2
#define LIBRARY_DATETXT	  __AMIGADATE__
#define LIBRARY_VERSTXT	 GSTR( LIBRARY_VERSION ) ".00" GSTR( LIBRARY_REVISION )

#if defined( _M68060 )
  #define LIBRARY_CPUTXT  " 060"
#elif defined( _M68040 )
  #define LIBRARY_CPUTXT  " 040"
#elif defined( _M68030 )
  #define LIBRARY_CPUTXT  " 030"
#elif defined( _M68020 )
  #define LIBRARY_CPUTXT  " 020"
#elif defined( __MORPHOS__ )
  #define LIBRARY_CPUTXT  " MorphOS"
#else
  #define LIBRARY_CPUTXT  " 000"
#endif

#if defined( __VBCC__ )
  #define LIBRARY_COMPILERTXT " vbcc"
#elif defined( __SASC )
  #define LIBRARY_COMPILERTXT " SAS/C"
#endif

#ifdef CROSS_TOOLCHAIN
  #define LIBRARY_HOSTTXT " cross"
#else
  #define LIBRARY_HOSTTXT " native"
#endif

#define LIBRARY_IDSTRING \
  LIBRARY_NAME " " LIBRARY_VERSTXT " " LIBRARY_DATETXT \
  LIBRARY_CPUTXT LIBRARY_COMPILERTXT LIBRARY_HOSTTXT
*/

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
  struct Library              * SysBase;
};

/******************************************************************************
 * Define your library's base type here, will be used in library.c.
 *****************************************************************************/
#define LIBRARY_TYPE      struct AmiGUS_MHI

/******************************************************************************
 * Your library's own base structure shall have its own include,
 * maybe together with your library specific functions.
 * Include it here!
 *****************************************************************************/
#include "amigus_mhi.h"

/******************************************************************************
 * Now go ahead and implement these functions in your library adapter code!
 *****************************************************************************/
LONG CustomLibInit( LIBRARY_TYPE * libBase, struct ExecBase * sysBase );
VOID CustomLibClose( LIBRARY_TYPE * libBase );

#endif /* LIBRARY_H */
