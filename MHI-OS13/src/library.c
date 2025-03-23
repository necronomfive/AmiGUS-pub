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
#include <exec/resident.h>
#include <exec/nodes.h>


#include "macros.h"
#include "config.h"
#include "compiler.h"

#include "amigus_mhi.h"

ASM(LONG) SAVEDS LibNull( VOID ) {
     return 0;
}
 
extern const char LibName[];
extern const char _LibVersionString[];
extern const APTR LibInitTab[];

static const struct Resident _00RomTag;
static const struct Resident _00RomTag = {
  RTC_MATCHWORD,
  ( struct Resident * ) &_00RomTag,
  ( APTR ) &_00RomTag + 1,
  RTF_AUTOINIT,
  LIB_VERSION,
  NT_LIBRARY,
  0,
  ( STRPTR ) LibName,
  ( STRPTR ) _LibVersionString,
  ( APTR ) LibInitTab
};

#define INITBYTE(offset,value)  0xe000,(UWORD) (offset),(UWORD) ((value)<<8)
#define INITWORD(offset,value)  0xd000,(UWORD) (offset),(UWORD) (value)
#define INITLONG(offset,value)  0xc000,(UWORD) (offset), \
                                (UWORD) ((value)>>16), \
                                (UWORD) ((value) & 0xffff)
#define INITSTRUCT(size,offset,value,count) \
                                (UWORD) (0xc000|(size<<12)|(count<<8)| \
                                ((UWORD) ((offset)>>16)), \
                                ((UWORD) (offset)) & 0xffff)

const char _LibVersionString[] = "$VER: " AMIGUS_MHI_VERSION "\r\n";
const char LibName[] = STR( LIB_FILE );

const APTR LibFunctions[] = {
  /* from libfuncs_impl.h */
  ( APTR ) LibOpen,
  ( APTR ) LibClose,
  ( APTR ) LibExpunge,
  ( APTR ) LibNull,
  /* insert APTRs for your function calls here */
  ( APTR ) MHIAllocDecoder, \
  ( APTR ) MHIFreeDecoder, \
  ( APTR ) MHIQueueBuffer, \
  ( APTR ) MHIGetEmpty, \
  ( APTR ) MHIGetStatus, \
  ( APTR ) MHIPlay, \
  ( APTR ) MHIStop, \
  ( APTR ) MHIPause, \
  ( APTR ) MHIQuery, \
  ( APTR ) MHISetParam,
  ( APTR ) -1
};

#define WORDINIT(_a_) UWORD _a_ ##W1; UWORD _a_ ##W2; UWORD _a_ ##ARG;
#define LONGINIT(_a_) UBYTE _a_ ##A1; UBYTE _a_ ##A2; ULONG _a_ ##ARG;
struct LibInitData
{
  WORDINIT(w1) 
  LONGINIT(l1)
  WORDINIT(w2) 
  WORDINIT(w3) 
  WORDINIT(w4) 
  LONGINIT(l2)
  ULONG end_initlist;
} LibInitializers =
{
  INITBYTE(     STRUCTOFFSET( Node,  ln_Type),         NT_LIBRARY),
  0x80, (UBYTE) ((LONG)STRUCTOFFSET( Node,  ln_Name)), (ULONG) &LibName[0],
  INITBYTE(     STRUCTOFFSET(Library,lib_Flags),       LIBF_SUMUSED|LIBF_CHANGED ),
  INITWORD(     STRUCTOFFSET(Library,lib_Version),     LIB_VERSION  ),
  INITWORD(     STRUCTOFFSET(Library,lib_Revision),    LIB_REVISION ),
  0x80, (UBYTE) ((LONG)STRUCTOFFSET(Library,lib_IdString)), (ULONG) &_LibVersionString,
  (ULONG) 0
};

const APTR LibInitTab[] = {
  (APTR) sizeof( BASETYPE ),
  (APTR) &LibFunctions,
  (APTR) &LibInitializers,
  (APTR) LibInit
};
