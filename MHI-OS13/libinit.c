/*
  libinit.c 

  (C) 2018 Henryk Richter <henryk.richter@gmx.net>

  initialization structures and data

*/
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/lists.h>
#include "version.h"
#include "macros.h"

#include "config.h"

#include "libfuncs_impl.h"
#include "SDI_mhi_protos.h"

#define xstr(a) str(a)
#define str(a) #a

#ifndef LIBEXTRA
#define LIBEXTRA
#endif

#ifdef __SASC
#define _STATIC_ static
#else
#define _STATIC_
#endif

#if 1
/* from exec/initializers.h */
// #define OFFSET(structName, structEntry) \
//                                 (&(((struct structName *) 0)->structEntry))
#define INITBYTE(offset,value)  0xe000,(UWORD) (offset),(UWORD) ((value)<<8)
#define INITWORD(offset,value)  0xd000,(UWORD) (offset),(UWORD) (value)
#define INITLONG(offset,value)  0xc000,(UWORD) (offset), \
                                (UWORD) ((value)>>16), \
                                (UWORD) ((value) & 0xffff)
#define INITSTRUCT(size,offset,value,count) \
                                (UWORD) (0xc000|(size<<12)|(count<<8)| \
                                ((UWORD) ((offset)>>16)), \
                                ((UWORD) (offset)) & 0xffff)
#else
#include <exec/initializers.h>
#endif

/* strings from version.h */
_STATIC_ const unsigned short hdr[] = { 0x2456,0x4552,0x3a20 }; /* "$VER: " */
const char *_LibVersionString = xstr(LIBNAME) " " xstr(LIBVERSION) "." xstr(LIBREVISION) " (" xstr(LIBDATE) ") " xstr(LIBEXTRA) "\r\n";
const char LibName[] = xstr(LIBNAME);

const APTR LibFunctions[] = {
	/* from libfuncs_impl.h */
	(APTR) LibOpen,
	(APTR) LibClose,
	(APTR) LibExpunge,
	(APTR) LibNull,
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
	(APTR) -1
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
	INITWORD(     STRUCTOFFSET(Library,lib_Version),     LIBVERSION  ),
	INITWORD(     STRUCTOFFSET(Library,lib_Revision),    LIBREVISION ),
	0x80, (UBYTE) ((LONG)STRUCTOFFSET(Library,lib_IdString)), (ULONG) &_LibVersionString,
	(ULONG) 0
};


const APTR LibInitTab[] = {
	(APTR) sizeof( BASETYPE ),
	(APTR) &LibFunctions,
	(APTR) &LibInitializers,
	(APTR) LibInit
};

#if 0
float blah2(float a )
{
	return a;
}
#endif
