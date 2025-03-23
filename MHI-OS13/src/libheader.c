/*
   Library Header - ROMTAG

   (C) 2018 Henryk Richter <henryk.richter@gmx.net>

   taken from version.h  
    -DLIBNAME=blah.device
    -DLIBVERSION=45
    -DLIBREVISION=36
    -DLIBDATE=2.12.2012

   this must be placed first in the object list for the linker

   The construct is broken down into 3 distinct files:
   - libheader.c
   - libinit.c
   - functions file(s), e.g. libfuncs_impl.c

*/

#include <exec/types.h>
#include <exec/resident.h>
#include <exec/nodes.h>
//#include <exec/initializers.h>
#include <exec/libraries.h>
#include "version.h"
#include "compiler.h"


#if 0
#define xstr(a) str(a)
#define str(a) #a
#endif

ASM LONG LibNull( void )
{
	return 0;
}


extern const char LibName[];
extern const char _LibVersionString[];
extern const APTR LibInitTab[];

static const struct Resident _00RomTag;
static const struct Resident _00RomTag = {
	RTC_MATCHWORD,
	( struct Resident* ) &_00RomTag,
	( struct Resident* ) &_00RomTag + 1,
	RTF_AUTOINIT,
	LIBVERSION,
	NT_LIBRARY,
	0,
	(char*)LibName,
	(char*)_LibVersionString,
	(APTR)LibInitTab
};


#ifdef __SASC
/* KLUDGE: SAS/C linker complains with warning 625 about "wrong math library".
   Point is: we don't need any implicit math library functions. This construct
   silences the linker.
*/
int __stdargs __fpinit(void);
float blah(float a )
{
	__fpinit();
	return a;
}
ASM int _XCEXIT( void )
{
  return 0;
}
#endif
