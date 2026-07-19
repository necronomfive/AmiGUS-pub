/*
 * This file is part of the amigus.library.
 *
 * amigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * amigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with amigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <exec/libraries.h>
#include <exec/nodes.h>
#include <libraries/dos.h>

#include <proto/exec.h>

#include "SDI_compiler.h"
#include "SDI_amigus_protos.h"

// This file lives before the library is set up completely...
// so it does not use the default library definitions, hence:
#define NO_BASE_REDEFINE
#include "amigus_private.h"

/******************************************************************************
 * OS required library basics - private macros.
 *****************************************************************************/

/**
 * Allow at least usage of exec functions below,
 * everything else is not initialized and not needed.
 */
#define SysBase base->SysBase

/*
 * Custom Version of exec/initializers.h, for 1.3 compatibility.
 */
#define WORDINIT( _a_ ) UWORD _a_ ##W1; UWORD _a_ ##W2; UWORD _a_ ##ARG;
#define LONGINIT( _a_ ) UBYTE _a_ ##A1; UBYTE _a_ ##A2; ULONG _a_ ##ARG;
#define INITBYTE( offset, value )  \
        0xe000, ( UWORD ) ( offset ), \
        ( UWORD ) (( value ) << 8 )
#define INITWORD( offset, value )  \
        0xd000, ( UWORD ) ( offset ), \
        ( UWORD ) ( value )
#define INITLONG( offset, value )  \
        0xc000, ( UWORD ) ( offset ), \
        ( UWORD ) (( value ) >> 16 ), ( UWORD ) (( value ) & 0xffff )

/******************************************************************************
 * OS required library basics - private function declarations.
 *****************************************************************************/

// 'cause otherwise VBCC does not swallow the ASM macros
typedef struct Library * LIB_PTR;

/**
 * Library initialization - run only once when library is put into memory.
 *
 * @param base     Pointer to the allocated library base structure.
 * @param seglist  List of library code segments
 * @param _SysBase Exec library pointer
 *
 * @return Pointer to the allocated library base structure,
 *         NULL otherwise.
 */
static ASM( LIB_PTR ) SAVEDS LibInit(
  REG( d0, struct BaseLibrary * base ),
  REG( a0, SEGLISTPTR seglist ),
  REG( a6, struct Library * _SysBase ));

/**
 * Library opening code - run every time when the library is OpenLibrary-ed.
 *
 * @param base Pointer to the allocated library base structure.
 *
 * @return Pointer to the allocated library base structure.
 */
static ASM( LIB_PTR ) SAVEDS LibOpen(
  REG( a6, struct BaseLibrary * base ));

/**
 * Library closing code - run every time when the library is CloseLibrary-ed.
 *
 * @param base Pointer to the allocated library base structure.
 *
 * @return NULL if the library is still opened at least once,
 *         return value of LibExpunge otherwise.
 */
static ASM( SEGLISTPTR ) SAVEDS LibClose(
  REG( a6, struct BaseLibrary * base ));

/**
 * Library extinction - run only once when library is erased from memory.
 *
 * @param base Pointer to the allocated library base structure.
 *
 * @return NULL if the library is still opened at least once,
 *         list of the code segments to erase/return to empty pool otherwise.
 */
static ASM( SEGLISTPTR ) SAVEDS LibExpunge(
  REG( a6, struct BaseLibrary * base ));

/******************************************************************************
 * OS required library basics - private data declarations.
 *****************************************************************************/

/**
 * Just the library name as C-string.
 */
extern const char LibName[];

/**
 * Full library version string to be printed by
 * version <library> full
 * via shell.
 */
extern const char _LibVersionString[];

/**
 * Library initialization table,
 * - Size of the libraries base structure -
 *   how much memory shall be allocated when the library is loaded?
 * - Library function list, terminated by -1.
 * - Library initializers, with the address of the wierd struct LibInitData.
 * - Pointer to the LibInit function.
 */
extern const APTR LibInitTab[];

/******************************************************************************
 * OS required library basics - private functions.
 *****************************************************************************/

/**
 * First function in an "executable" is called on startup by AmigaOS.
 * Therefore an empty function prevents disasters here.
 *
 * @return 0 as OS return code.
 */
ASM( LONG ) SAVEDS LibNull( VOID ) {

    return 0;
}

/******************************************************************************
 * OS required library basics - private data definitions.
 *****************************************************************************/

/**
 * Feeds the OS's library loader with the required information.
 * May not be omitted and shall not be optimized away either!
 */
static const struct Resident _00RomTag = {
  RTC_MATCHWORD,
  ( struct Resident * ) &_00RomTag,
  ( APTR ) ( &_00RomTag + 1 ),
  RTF_AUTOINIT,
  LIBRARY_VERSION,
  NT_LIBRARY,
  0,
  ( STRPTR ) LibName,
  ( STRPTR ) _LibVersionString,
  ( APTR ) LibInitTab
};

// As per above:
const char _LibVersionString[] = "$VER: " LIBRARY_IDSTRING "\r\n";

// As per above:
const char LibName[] = STR( LIBRARY_NAME );

/**
 * Table of all functions to be externally defined by the library.
 * With minimum functionality required by the OS and
 * finally the functions defined by the actual library to serve some purpose.
 * Terminated by -1.
 */
const APTR LibFunctions[] = {
  /* Basic library open, close, flush functions */
  ( APTR ) LibOpen,
  ( APTR ) LibClose,
  ( APTR ) LibExpunge,
  ( APTR ) LibNull,
  /* Real library specific functions */
  LIBRARY_FUNCTIONS,
  /* End marker */
  ( APTR ) -1
};

/**
 * Mysterious library init structure.
 * Do not touch - nothing to win here!
 */
struct LibInitData {

  WORDINIT( w1 )
  LONGINIT( l1 )
  WORDINIT( w2 )
  WORDINIT( w3 )
  WORDINIT( w4 )
  LONGINIT( l2 )
  ULONG end_initlist;

} LibInitializers = {

  INITBYTE( OFFSET( Node,  ln_Type), NT_LIBRARY ),
  0x80,
  ( UBYTE ) (( LONG ) OFFSET( Node,  ln_Name)),
  ( ULONG ) &LibName[ 0 ],
  INITBYTE( OFFSET( Library, lib_Flags ), LIBF_SUMUSED | LIBF_CHANGED ),
  INITWORD( OFFSET( Library, lib_Version ), LIBRARY_VERSION  ),
  INITWORD( OFFSET( Library, lib_Revision ), LIBRARY_REVISION ),
  0x80,
  ( UBYTE ) (( LONG ) OFFSET( Library, lib_IdString )),
  ( ULONG ) &_LibVersionString,
  ( ULONG ) 0
};

/**
 * Mysterious library init table.
 * Do not touch - nothing to win here!
 */
const APTR LibInitTab[] = {

  ( APTR ) sizeof( LIBRARY_TYPE ),
  ( APTR ) &LibFunctions,
  ( APTR ) &LibInitializers,
  ( APTR ) LibInit
};

/******************************************************************************
 * OS required library basics - private function definitions.
 *****************************************************************************/

static ASM( LIB_PTR ) SAVEDS LibInit(
  REG( d0, struct BaseLibrary * base ),
  REG( a0, SEGLISTPTR seglist ),
  REG( a6, struct Library * _SysBase )) {

  ULONG p = ( ULONG ) base;
  ULONG t = p + sizeof( LIBRARY_TYPE );

  if ( !p ) {

    return NULL;
  }

  p += sizeof( struct Library );
  while ( p < t ) {
    
    *(( UBYTE * ) p++ ) = 0;
  }

  SysBase = *(( struct ExecBase ** ) 4UL );

  InitSemaphore( &base->LockSemaphore );

  base->SegList = seglist;
  base->LibNode.lib_IdString = ( APTR ) _LibVersionString;

  if ( !CustomLibInit(( LIBRARY_TYPE * ) base,
                      ( struct ExecBase * ) SysBase )) {
    
    return ( LIB_PTR ) base;
  }

  CustomLibClose(( LIBRARY_TYPE * ) base );

  /* Free the vector table and the library data */
  FreeMem(( APTR )(( ULONG ) base - ( ULONG )( base->LibNode.lib_NegSize )),
          base->LibNode.lib_NegSize + base->LibNode.lib_PosSize );

  return NULL;
}

static ASM( LIB_PTR ) SAVEDS LibOpen(
  REG( a6, struct BaseLibrary * base )) {

  ObtainSemaphore( &base->LockSemaphore );

  base->LibNode.lib_Flags &= ~LIBF_DELEXP;
  base->LibNode.lib_OpenCnt++;

  ReleaseSemaphore( &base->LockSemaphore );

  return ( struct Library * ) base;
}

static ASM( SEGLISTPTR ) SAVEDS LibClose(
  REG( a6, struct BaseLibrary * base )) {

  ObtainSemaphore( &base->LockSemaphore );

  if ( base->LibNode.lib_OpenCnt ) {

    --base->LibNode.lib_OpenCnt;
  }

  ReleaseSemaphore( &base->LockSemaphore );
  if (( base->LibNode.lib_Flags & LIBF_DELEXP ) &&
      ( !base->LibNode.lib_OpenCnt )) {

    return LibExpunge( base );
  }

  return NULL;
}

static ASM( SEGLISTPTR ) SAVEDS LibExpunge(
  REG( a6, struct BaseLibrary * base )) {

  SEGLISTPTR ret;

  if ( base->LibNode.lib_OpenCnt ) {

    base->LibNode.lib_Flags |= LIBF_DELEXP;
    return NULL;
  }

  CustomLibClose(( LIBRARY_TYPE * ) base );

  Forbid();
  Remove(( struct Node * ) base ); /* Lib no longer in system */
  Permit();

  ret = base->SegList;
  
  FreeMem(( APTR )(( ULONG ) base - ( ULONG )( base->LibNode.lib_NegSize )),
          base->LibNode.lib_NegSize + base->LibNode.lib_PosSize );

  return ret;
}
