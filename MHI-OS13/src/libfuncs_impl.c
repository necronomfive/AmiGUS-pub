/*
  file:   libfuncs_impl.c

  author:  Henryk Richter <henryk.richter@gmx.net>

*/
#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
//#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/timer.h>
//#include <dos/exall.h>

#define __CONSTLIBBASEDECL__ /* some GCC targeted includes carry this attribute */ 
#define _MYLIBRARY_FUNCS_INTERNAL
#include "libfuncs_impl.h"
#include "compiler.h"

#include "macros.h"
#include "config.h"
#include "debug.h"

void StringCopy( STRPTR d, STRPTR s )
{
	while( *s )
	{
		*d++ = *s++;
	}
	*d = 0;
}

void StringCat( STRPTR d, STRPTR s )
{
	while( *++d ){}

	while( *s )
	{
		*d++ = *s++;
	}
	*d = 0;
}

/*
  this is just bzero()
*/
void MemClear( void *ptr, LONG bytes )
{
        UBYTE *p = (UBYTE*)ptr;

        while( bytes-- )
                *p++ = 0;
}



/*

   Library init call

*/
ASM SAVEDS struct Library *LibInit( ASMR(d0) BASETYPE *example_libbase  ASMREG(d0),
                              ASMR(a0) BPTR seglist              ASMREG(a0),
                              ASMR(a6) struct Library *_SysBase  ASMREG(a6) )
{
	UBYTE *p = (UBYTE*)example_libbase;
	extern char *_LibVersionString;

	if( !p )
		return (0);

	MemClear( p + sizeof(struct Library), sizeof(struct example_libbase) - sizeof(struct Library) );

	SysBase = *((struct Library **)4UL); /* copy ExecBase ptr to new struct */

	D(("Before Libs open\n"));

	InitSemaphore( &example_libbase->LockSemaphore );

	example_libbase->seglist    = seglist;
	example_libbase->libnode.lib_IdString = ( APTR ) _LibVersionString;

	/* config.c uses memory pools, require kick 3.0 */
	/*example_libbase->utilitylib = OpenLibrary( (STRPTR)"utility.library", 39 );*/
	example_libbase->doslib     = OpenLibrary( (STRPTR)"dos.library", 34       );
	/*example_libbase->mathieeesingbas = OpenLibrary( (STRPTR)"mathieeesingbas.library",37);*/

	D(("After Libs open\n"));

	return (struct Library *)example_libbase;
}



/*

   Library open Call

*/
ASM SAVEDS struct Library * LibOpen( ASMR(a6) struct example_libbase *example_libbase ASMREG(a6) )
{
	ObtainSemaphore( &example_libbase->LockSemaphore );

	example_libbase->libnode.lib_Flags &= ~LIBF_DELEXP;
	example_libbase->libnode.lib_OpenCnt++;

	ReleaseSemaphore( &example_libbase->LockSemaphore );

	return (struct Library*)example_libbase;
}



/*

   Library close Call

*/
ASM SAVEDS BPTR LibClose( ASMR(a6) struct example_libbase *example_libbase ASMREG(a6) )
{

	ObtainSemaphore( &example_libbase->LockSemaphore );

	if( example_libbase->libnode.lib_OpenCnt > 0 )
		example_libbase->libnode.lib_OpenCnt--;

	ReleaseSemaphore( &example_libbase->LockSemaphore );

	if( (example_libbase->libnode.lib_Flags & LIBF_DELEXP) &&
	    (example_libbase->libnode.lib_OpenCnt == 0)
	  )
		return LibExpunge( example_libbase );

	return (0);
}



/*

   Library cleanup 

*/
ASM SAVEDS BPTR LibExpunge( ASMR(a6) struct example_libbase *example_libbase ASMREG(a6) )
{
	BPTR ret;

	if( example_libbase->libnode.lib_OpenCnt != 0 )
	{
		example_libbase->libnode.lib_Flags |= LIBF_DELEXP;
		return(0);
	}

	Forbid();

	REMOVE( example_libbase ); /* Lib no longer in system */

	Permit();

	ret = example_libbase->seglist;

	FreeMem( (APTR)( (ULONG)example_libbase - (ULONG)(example_libbase->libnode.lib_NegSize) ),
	         example_libbase->libnode.lib_NegSize + example_libbase->libnode.lib_PosSize );
	
	return ret;
}




