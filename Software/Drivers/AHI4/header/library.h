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

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <libraries/dos.h>

/**
 * So what is that library.h/.c thing about?
 * - This is an AmigaOS v34+ compatible as-easy-as-possible usable
 *   library skeleton.
 * - Just plug-in
 * -- your LIBRARY_FUNCTIONS,
 * -- your LIBRARY_TYPE,
 * -- your library's main include file,
 * -- provide LIBRARY_NAME, LIBRARY_VERSI0N, LIBRARY_REVISION,
 *    and LIBRARY_IDSTRING however you like and you are good to go.
 * - Limitations:
 * -- Library filename and LIBRARY_NAME need to be case sensitive the same,
 * -- In OS v34, libraries have to be in LIBS: - not even subfolders,
 * -- struct BaseLibrary needs to be the first element in your library's
 *    lib structure.
 */

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
 * Define your library's base type here, will be used in library.c.
 *****************************************************************************/
#define LIBRARY_TYPE      struct AmiGUS_AHI_Base

LIBRARY_TYPE;

/******************************************************************************
 * Define your library's properties here,
 * will be used in library.c.
 *****************************************************************************/

#define STR_VALUE(x)      #x
#define STR(x)            STR_VALUE(x)

#define LIBRARY_NAME      LIB_FILE
#define LIBRARY_VERSION   LIB_VERSION
#define LIBRARY_REVISION  LIB_REVISION
#define LIBRARY_IDSTRING  STR( LIB_FILE )" "                         \
                          STR( LIB_VERSION )".0"STR( LIB_REVISION ) \
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

  struct Library                LibNode;       // Standard library node
  UWORD                         Unused0;       // Padding for better alignment
  SEGLISTPTR                    SegList;       // List of library code segments
  struct SignalSemaphore        LockSemaphore; // Prevents lib race conditions
  struct ExecBase             * SysBase;       // Pointer to exec library
};

/******************************************************************************
 * Your library's own base structure shall have its own include,
 * maybe together with your library specific functions.
 * Include it here and in library.c!
 *****************************************************************************/

 #include "amigus_ahi_sub.h"

/******************************************************************************
 * Now go ahead and implement these functions in your library adapter code!
 *****************************************************************************/

/**
 * Hook to plug your own library's initialization code into.
 *
 * @param base Pointer to the allocated library base structure.
 * @param sysBase Pointer to exec library as needed before.
 *
 * @return 0 expected in success case,
 *         anything else will be treated as failure.
 */
LONG CustomLibInit( LIBRARY_TYPE * base, struct ExecBase * sysBase );

/**
 * Hook to plug your own library's cleanup, de-initialization,
 * or close code into.
 * Closes all the libraries opened by CustomLibInit()
 *
 * @param base Pointer to the allocated library base structure.
 */
VOID CustomLibClose( LIBRARY_TYPE * base );

#endif /* LIBRARY_H */
