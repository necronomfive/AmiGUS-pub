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
 
#ifndef LIBRARY_C
#define LIBRARY_C

/* This file cannot cope with BASE_REDEFINE, blocking that permanently here. */
#define NO_BASE_REDEFINE

#include <exec/execbase.h>
#include <exec/resident.h>
#include <proto/exec.h>

#include "SDI_compiler.h"
#include "library.h"

#ifdef __MORPHOS__
#ifndef RTF_PPC
#define RTF_PPC (1<<3) /* rt_Init points to a PPC function */
#endif
#ifndef FUNCARRAY_32BIT_QUICK_NATIVE
#define FUNCARRAY_32BIT_QUICK_NATIVE 0xFFFBFFFB
#endif
/* To tell the loader that this is a new emulppc elf and not
 * one for the ppc.library. */
ULONG __amigappc__=1;
#endif

/************************************************************************/

/* First executable routine of this library; must return an error
   to the unsuspecting caller */
LONG ReturnError( VOID ) {

  return -1;
}

/*****************************************************************************/

/* natural aligned! */
struct LibInitData {

  UBYTE i_Type;     UBYTE o_Type;                                     // 2 byte
  UBYTE  d_Type;    UBYTE p_Type;                                     // 2 byte
  UWORD i_Name;     UWORD o_Name;     STRPTR d_Name;                  // 8 byte
  UBYTE i_Flags;    UBYTE o_Flags;                                    // 2 byte
  UBYTE  d_Flags;   UBYTE p_Flags;                                    // 2 byte
  UBYTE i_Version;  UBYTE o_Version;  UWORD  d_Version;               // 4 byte
  UBYTE i_Revision; UBYTE o_Revision; UWORD  d_Revision;              // 4 byte
  UWORD i_IdString; UWORD o_IdString; STRPTR d_IdString;              // 8 byte
  ULONG endmark;                                                      // 4 byte
};

/*****************************************************************************/
extern const ULONG LibInitTable[4]; /* the prototype */

/* The library loader looks for this marker in the memory
   the library code and data will occupy. It is responsible
   setting up the Library base data structure. */
const struct Resident RomTag = {

  RTC_MATCHWORD,                   /* Marker value.                          */
  (struct Resident *)&RomTag,      /* This points back to itself.            */
  (struct Resident *)LibInitTable, /* Points somewhere behind this marker.   */
#ifdef __MORPHOS__
  RTF_PPC|
#endif
  RTF_AUTOINIT,                    /* Set up shall be according to table.    */
  LIBRARY_VERSION,                 /* Version of this Library.               */
  NT_LIBRARY,                      /* Defines this module as a Library.      */
  0,                               /* (Unused) Initialization priority.      */
  LIBRARY_NAME,                    /* Points to the name of the Library.     */
  LIBRARY_IDSTRING,                /* Identification string of this Library. */
  (APTR)&LibInitTable              /* Table is for initializing the Library. */
};

/*****************************************************************************/

/* The mandatory reserved library function */
static ULONG LibReserved( VOID ) {

  return 0;
}

/* Open the library, as called via OpenLibrary() */
static ASM(struct Library *) LibOpen( REG(a6, struct BaseLibrary * base) ) {

  /* Prevent delayed expunge and increment opencnt */
  base->LibNode.lib_Flags &= ~LIBF_DELEXP;
  base->LibNode.lib_OpenCnt++;

  return &base->LibNode;
}

/* Expunge the library, remove it from memory */
static ASM(SEGLISTPTR) LibExpunge( REG(a6, struct BaseLibrary * base) ) {

  if(!base->LibNode.lib_OpenCnt) {

    SEGLISTPTR seglist = base->SegList;
    
    CustomLibClose( base );
    
    /* Remove the library from the public list */
    Remove( (struct Node *) base );

    /* Free the vector table and the library data */
    FreeMem(
      (STRPTR) base - base->LibNode.lib_NegSize,
      base->LibNode.lib_NegSize + base->LibNode.lib_PosSize );

    return seglist;

  } else {

    base->LibNode.lib_Flags |= LIBF_DELEXP;
  }
  /* Return the segment pointer, if any */
  return 0;
}

/* Close the library, as called by CloseLibrary() */
static ASM(SEGLISTPTR) LibClose( REG(a6, struct BaseLibrary * base) ) {

  if( !(--base->LibNode.lib_OpenCnt) ) {

    if( base->LibNode.lib_Flags & LIBF_DELEXP ) {

      return LibExpunge( base );
    }
  }
  return 0;
}

/* Initialize library */
#ifdef __MORPHOS__
static struct Library * LibInit(
  struct BaseLibrary * base,
  SEGLISTPTR seglist, 
  struct ExecBase *SysBase) {
#else
static ASM(struct Library *) LibInit(
  REG(a0, SEGLISTPTR seglist),
  REG(d0, struct BaseLibrary * base),
  REG(a6, struct ExecBase * sysBase) ) {
#endif

  /* Remember stuff */
  base->SegList = seglist;
  if ( !CustomLibInit( base, sysBase ) ) {
    
    return &base->LibNode;
  }

  CustomLibClose( base );

  /* Free the vector table and the library data */
  FreeMem(
      (STRPTR) base - base->LibNode.lib_NegSize,
      base->LibNode.lib_NegSize + base->LibNode.lib_PosSize );

  return 0;
}

/************************************************************************/

/*
 * This is the table of functions that make up the library. The first
 * four are mandatory, everything following it are user callable
 * routines. The table is terminated by the value -1.
 */
static const APTR LibVectors[] = {

#ifdef __MORPHOS__
  (APTR) FUNCARRAY_32BIT_QUICK_NATIVE,
#endif
  (APTR) LibOpen,
  (APTR) LibClose,
  (APTR) LibExpunge,
  (APTR) LibReserved,

  LIBRARY_FUNCTIONS,

  (APTR) -1
};

static const struct LibInitData LibInitData = {

  0xA0,   (UBYTE) OFFSET(Node,    ln_Type),      NT_LIBRARY,                0,
  0xC000, (UBYTE) OFFSET(Node,    ln_Name),      LIBRARY_NAME,
  0xA0,   (UBYTE) OFFSET(Library, lib_Flags),    LIBF_SUMUSED|LIBF_CHANGED, 0,
  0x90,   (UBYTE) OFFSET(Library, lib_Version),  LIBRARY_VERSION,
  0x90,   (UBYTE) OFFSET(Library, lib_Revision), LIBRARY_REVISION,
  0xC000, (UBYTE) OFFSET(Library, lib_IdString), LIBRARY_IDSTRING,
  0
};

/*
 * The following data structures and data are responsible for
 * setting up the Library base data structure and the library
 * function vector.
 */
const ULONG LibInitTable[4] = {

  (ULONG)sizeof( LIBRARY_TYPE ),    /* Size of the base data structure */
  (ULONG)LibVectors,                /* Points to the function vector */
  (ULONG)&LibInitData,              /* Library base data structure table */
  (ULONG)LibInit                    /* Address of the library setup routine */
};

#endif /* LIBRARY_C */
