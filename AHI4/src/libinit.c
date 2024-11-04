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
 
#ifndef LIBINIT_C
#define LIBINIT_C

/* This file cannot cope with BASE_REDEFINE, blocking that permanently here. */
#define NO_BASE_REDEFINE
#include "amigus_private.h"
#include "debug.h"
#include "support.h"

#include <exec/execbase.h>
#include <exec/resident.h>
#include <intuition/intuitionbase.h>
#include <libraries/expansionbase.h>
#include <proto/exec.h>
#include <proto/utility.h>

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
LONG ReturnError(void)
{
  return -1;
}

/************************************************************************/

/* natural aligned! */
struct LibInitData {
 UBYTE i_Type;     UBYTE o_Type;     UBYTE  d_Type;	UBYTE p_Type;
 UWORD i_Name;     UWORD o_Name;     STRPTR d_Name;
 UBYTE i_Flags;    UBYTE o_Flags;    UBYTE  d_Flags;	UBYTE p_Flags;
 UBYTE i_Version;  UBYTE o_Version;  UWORD  d_Version;
 UBYTE i_Revision; UBYTE o_Revision; UWORD  d_Revision;
 UWORD i_IdString; UWORD o_IdString; STRPTR d_IdString;
 ULONG endmark;
};

/************************************************************************/
extern const ULONG LibInitTable[4]; /* the prototype */

/* The library loader looks for this marker in the memory
   the library code and data will occupy. It is responsible
   setting up the Library base data structure. */
const struct Resident RomTag = {
  RTC_MATCHWORD,                   /* Marker value. */
  (struct Resident *)&RomTag,      /* This points back to itself. */
  (struct Resident *)LibInitTable, /* This points somewhere behind this marker. */
#ifdef __MORPHOS__
  RTF_PPC|
#endif
  RTF_AUTOINIT,                    /* The Library should be set up according to the given table. */
  VERSION,                         /* The version of this Library. */
  NT_LIBRARY,                      /* This defines this module as a Library. */
  0,                               /* Initialization priority of this Library; unused. */
  LIBNAME,                         /* Points to the name of the Library. */
  IDSTRING,                        /* The identification string of this Library. */
  (APTR)&LibInitTable              /* This table is for initializing the Library. */
};

/************************************************************************/

/* The mandatory reserved library function */
static ULONG LibReserved(void) {

  return 0;
}

/* Open the library, as called via OpenLibrary() */
static ASM(struct Library *) LibOpen(
  REG(a6, struct AmiGUSBasePrivate * amiGUSBase)) {

  /* Prevent delayed expunge and increment opencnt */
  amiGUSBase->agb_LibNode.lib_Flags &= ~LIBF_DELEXP;
  amiGUSBase->agb_LibNode.lib_OpenCnt++;

  return &amiGUSBase->agb_LibNode;
}

#ifdef BASE_GLOBAL
struct ExecBase          * SysBase           = 0;
struct DosLibrary        * DOSBase           = 0;
struct IntuitionBase     * IntuitionBase     = 0;
struct Library           * UtilityBase       = 0;
struct Library           * ExpansionBase     = 0;
struct Device            * TimerBase         = 0;
struct AmiGUSBasePrivate * AmiGUSBase        = 0;

static void MakeGlobalLibs(
    struct AmiGUSBasePrivate * amiGUSBase) {


  DOSBase       = amiGUSBase->agb_DOSBase;
  IntuitionBase = amiGUSBase->agb_IntuitionBase;
  UtilityBase   = amiGUSBase->agb_UtilityBase;
  ExpansionBase = amiGUSBase->agb_ExpansionBase;
TimerBase = amiGUSBase->TimerBase;
  AmiGUSBase    = amiGUSBase;
}

static void MakeGlobalSys(
    struct AmiGUSBasePrivate *amiGUSBase) {

  SysBase = amiGUSBase->agb_SysBase;
}
#endif

/* Closes all the libraries opened by LibInit() */
static void CloseLibraries(
    struct AmiGUSBasePrivate * AmiGUSBase) {

#ifndef BASE_GLOBAL
  struct ExecBase *SysBase = AmiGUSBase->agb_SysBase;
#endif

if(AmiGUSBase->TimerBase)
  CloseDevice(AmiGUSBase->TimerRequest);
if(AmiGUSBase->TimerRequest)
  FreeMem(AmiGUSBase->TimerRequest, sizeof(struct IORequest));


  if(AmiGUSBase->agb_DOSBase)
    CloseLibrary((struct Library *) AmiGUSBase->agb_DOSBase);
  if(AmiGUSBase->agb_IntuitionBase)
    CloseLibrary((struct Library *) AmiGUSBase->agb_IntuitionBase);
  if(AmiGUSBase->agb_UtilityBase)
    CloseLibrary((struct Library *) AmiGUSBase->agb_UtilityBase);
  if(AmiGUSBase->agb_ExpansionBase)
    CloseLibrary((struct Library *) AmiGUSBase->agb_ExpansionBase);
}

/* Expunge the library, remove it from memory */
static ASM(SEGLISTPTR) LibExpunge(
  REG(a6, struct AmiGUSBasePrivate * amiGUSBase)) {

#ifndef BASE_GLOBAL
  struct ExecBase *SysBase = amiGUSBase->agb_SysBase;
#endif

  if(!amiGUSBase->agb_LibNode.lib_OpenCnt) {

    SEGLISTPTR seglist;

    seglist = amiGUSBase->agb_SegList;

    CloseLibraries(amiGUSBase);

    /* Remove the library from the public list */
    Remove((struct Node *) amiGUSBase);

    /* Free the vector table and the library data */
    FreeMem((STRPTR) amiGUSBase - amiGUSBase->agb_LibNode.lib_NegSize,
    amiGUSBase->agb_LibNode.lib_NegSize +
    amiGUSBase->agb_LibNode.lib_PosSize);

    return seglist;
  }
  else
    amiGUSBase->agb_LibNode.lib_Flags |= LIBF_DELEXP;

  /* Return the segment pointer, if any */
  return 0;
}

/* Close the library, as called by CloseLibrary() */
static ASM(SEGLISTPTR) LibClose(
  REG(a6, struct AmiGUSBasePrivate * amiGUSBase)) {

  if(!(--amiGUSBase->agb_LibNode.lib_OpenCnt)) {

    if(amiGUSBase->agb_LibNode.lib_Flags & LIBF_DELEXP)
      return LibExpunge(amiGUSBase);
  }
  return 0;
}

/* Initialize library */
#ifdef __MORPHOS__
static struct Library * LibInit(
  struct AmiGUSBasePrivate * amiGUSBase,
  SEGLISTPTR seglist, 
  struct ExecBase *SysBase) {
#else
static ASM(struct Library *) LibInit(
  REG(a0, SEGLISTPTR seglist),
  REG(d0, struct AmiGUSBasePrivate * amiGUSBase),
  REG(a6, struct ExecBase *SysBase)) {
#endif

#ifdef _M68060
  if(!(SysBase->AttnFlags & AFF_68060))
    return 0;
#elif defined (_M68040)
  if(!(SysBase->AttnFlags & AFF_68040))
    return 0;
#elif defined (_M68030)
  if(!(SysBase->AttnFlags & AFF_68030))
    return 0;
#elif defined (_M68020)
  if(!(SysBase->AttnFlags & AFF_68020))
    return 0;
#endif

  /* Remember stuff */
  amiGUSBase->agb_SegList = seglist;
  amiGUSBase->agb_SysBase = SysBase;

#ifdef BASE_GLOBAL
  MakeGlobalSys(amiGUSBase);
#endif

  if((amiGUSBase->agb_DOSBase = (struct DosLibrary *) OpenLibrary(DOSNAME, 37)))
  {
    if((amiGUSBase->agb_IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 37)))
    {
      if((amiGUSBase->agb_UtilityBase = (struct Library *) OpenLibrary("utility.library", 37)))
      {
        if((amiGUSBase->agb_ExpansionBase = (struct Library *) OpenLibrary("expansion.library", 37)))
        {
          LONG error;

amiGUSBase->TimerRequest = AllocMem(
  sizeof(struct IORequest),
  MEMF_ANY | MEMF_CLEAR);
OpenDevice("timer.device", 0, amiGUSBase->TimerRequest, 0);
amiGUSBase->TimerBase = amiGUSBase->TimerRequest->io_Device;


#ifdef BASE_GLOBAL
          MakeGlobalLibs(amiGUSBase);
#endif
          LOG_D(("D: AmiGUS base ready @ 0x%08lx\n", AmiGUSBase));

          error = FindAmiGUS(amiGUSBase);
          if ( error ) {

            DisplayError(error);
          }

          return &amiGUSBase->agb_LibNode;
        }
      }
    }
    CloseLibraries(amiGUSBase);
  }

  /* Free the vector table and the library data */
  FreeMem(
      (STRPTR) amiGUSBase - amiGUSBase->agb_LibNode.lib_NegSize,
      amiGUSBase->agb_LibNode.lib_NegSize
          + amiGUSBase->agb_LibNode.lib_PosSize);

  return 0;
}

/************************************************************************/

/* This is the table of functions that make up the library. The first
   four are mandatory, everything following it are user callable
   routines. The table is terminated by the value -1. */

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
 0xC000, (UBYTE) OFFSET(Node,    ln_Name),      LIBNAME,
 0xA0,   (UBYTE) OFFSET(Library, lib_Flags),    LIBF_SUMUSED|LIBF_CHANGED, 0,
 0x90,   (UBYTE) OFFSET(Library, lib_Version),  VERSION,
 0x90,   (UBYTE) OFFSET(Library, lib_Revision), REVISION,
 0xC000, (UBYTE) OFFSET(Library, lib_IdString), IDSTRING,
 0
};

/* The following data structures and data are responsible for
   setting up the Library base data structure and the library
   function vector.
*/
const ULONG LibInitTable[4] = {
  (ULONG)sizeof(
      struct AmiGUSBasePrivate),    /* Size of the base data structure */
  (ULONG)LibVectors,                /* Points to the function vector */
  (ULONG)&LibInitData,              /* Library base data structure setup table */
  (ULONG)LibInit                    /* The address of the routine to do the setup */
};

#endif /* LIBINIT_C */
