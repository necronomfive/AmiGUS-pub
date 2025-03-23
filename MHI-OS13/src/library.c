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

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/nodes.h>
#include <exec/resident.h>
#include <exec/semaphores.h>

#include <libraries/dos.h>

#include <proto/exec.h>

// This file lives before the library is set up completely...
// so it does not use the default library definitions, hence:
#define NO_BASE_REDEFINE
#include "amigus_mhi.h"

/* Private forward declarations */

// 'cause otherwise VBCC does not swallow the ASM macros
typedef struct Library * LIB_PTR;

static ASM( LIB_PTR ) SAVEDS LibInit(
  REG( d0, struct BaseLibrary * base ),
  REG( a0, SEGLISTPTR seglist ),
  REG( a6, struct Library * _SysBase ));

static ASM( LIB_PTR ) SAVEDS LibOpen(
  REG( a6, struct BaseLibrary * base ));

static ASM( SEGLISTPTR ) SAVEDS LibClose(
  REG( a6, struct BaseLibrary * base ));

static ASM( SEGLISTPTR ) SAVEDS LibExpunge(
  REG( a6, struct BaseLibrary * base ));

#define SysBase base->SysBase

////////////////////////////////////////////////////////////////////7

ASM( LONG ) SAVEDS LibNull( VOID ) {

    return 0;
}
 
extern const char LibName[];
extern const char _LibVersionString[];
extern const APTR LibInitTab[];

static const struct Resident _00RomTag = {
  RTC_MATCHWORD,
  ( struct Resident * ) &_00RomTag,
  ( APTR ) ( &_00RomTag + 1 ),
  RTF_AUTOINIT,
  LIB_VERSION,
  NT_LIBRARY,
  0,
  ( STRPTR ) LibName,
  ( STRPTR ) _LibVersionString,
  ( APTR ) LibInitTab
};

const char _LibVersionString[] = "$VER: " AMIGUS_MHI_VERSION "\r\n";
const char LibName[] = STR( LIB_FILE );

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
  INITWORD( OFFSET( Library, lib_Version ), LIB_VERSION  ),
  INITWORD( OFFSET( Library, lib_Revision ), LIB_REVISION ),
  0x80,
  ( UBYTE ) (( LONG ) OFFSET( Library, lib_IdString )),
  ( ULONG ) &_LibVersionString,
  ( ULONG ) 0
};

const APTR LibInitTab[] = {

  ( APTR ) sizeof( LIBRARY_TYPE ),
  ( APTR ) &LibFunctions,
  ( APTR ) &LibInitializers,
  ( APTR ) LibInit
};

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

  SysBase = *(( struct Library ** ) 4UL );

  InitSemaphore( &base->LockSemaphore );

  base->SegList = seglist;
  base->LibNode.lib_IdString = ( APTR ) _LibVersionString;

  return ( struct Library * ) base;
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

  Forbid();
  Remove(( struct Node * ) base ); /* Lib no longer in system */
  Permit();

  ret = base->SegList;
  
  FreeMem(( APTR )(( ULONG ) base - ( ULONG )( base->LibNode.lib_NegSize )),
          base->LibNode.lib_NegSize + base->LibNode.lib_PosSize );

  return ret;
}
