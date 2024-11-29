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

#ifndef LIBRARY_H
#define LIBRARY_H

#include <dos/dos.h>
#include <exec/libraries.h>

/******************************************************************************
 * Define your library's public functions here,
 * will be used in library.c.
 *****************************************************************************/

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

/******************************************************************************
 * Define your library's properties here,
 * will be used in library.c.
 *****************************************************************************/
#define LIBRARY_NAME      "AmiGUS.audio"
#define LIBRARY_VERSION   4
#define LIBRARY_REVISION  1
#define LIBRARY_DATETXT	  __AMIGADATE__
#define LIBRARY_VERSTXT	  "4.003"

#ifdef _M68060
  #define LIBRARY_ADDTXT  " 060"
#elif defined(_M68040)
  #define LIBRARY_ADDTXT  " 040"
#elif defined(_M68030)
  #define LIBRARY_ADDTXT  " 030"
#elif defined(_M68020)
  #define LIBRARY_ADDTXT  " 020"
#elif defined(__MORPHOS__)
  #define LIBRARY_ADDTXT  " MorphOS"
#else
  #define LIBRARY_ADDTXT  ""
#endif

#define LIBRARY_IDSTRING \
  LIBRARY_NAME " " LIBRARY_VERSTXT " " LIBRARY_DATETXT LIBRARY_ADDTXT "\r\n"

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
};

/******************************************************************************
 * Define your library's base type here, will be used in library.c.
 *****************************************************************************/
#define LIBRARY_TYPE      struct AmiGUSBasePrivate

/******************************************************************************
 * Your library's own base structure shall have its own include,
 * maybe together with your library specific functions.
 * Include it here!
 *****************************************************************************/
#include "amigus_private.h"

/******************************************************************************
 * Now go ahead and implement these functions in your library adapter code!
 *****************************************************************************/
LONG CustomLibInit( struct BaseLibrary * base, struct ExecBase * sysBase );
VOID CustomLibClose( struct BaseLibrary * base );

#endif /* LIBRARY_H */
