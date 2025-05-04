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

#ifndef LIBRARY_H
#define LIBRARY_H

#include <dos/dos.h>
#include <exec/libraries.h>

/* Need 2 staged helpers to concat version strings and ints together... :/   */
#define GSTR_HELPER( x ) #x
#define GSTR( x )        GSTR_HELPER( x )

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
#define LIBRARY_REVISION  17
#define LIBRARY_DATETXT	  __AMIGADATE__
#define LIBRARY_VERSTXT	 GSTR( LIBRARY_VERSION ) ".0" GSTR( LIBRARY_REVISION )

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
  LIBRARY_CPUTXT LIBRARY_COMPILERTXT LIBRARY_HOSTTXT "\r\n"

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
#define LIBRARY_TYPE      struct AmiGUSBase

/******************************************************************************
 * Your library's own base structure shall have its own include,
 * maybe together with your library specific functions.
 * Include it here!
 *****************************************************************************/
#include "amigus_ahi_sub.h"

/******************************************************************************
 * Now go ahead and implement these functions in your library adapter code!
 *****************************************************************************/
LONG CustomLibInit( struct BaseLibrary * base, struct ExecBase * sysBase );
VOID CustomLibClose( struct BaseLibrary * base );

#endif /* LIBRARY_H */
