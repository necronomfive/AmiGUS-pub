#include <proto/exec.h>
#include <exec/execbase.h>
#include <stdio.h>

void FlushLib( char *name );

int main(int argc, char** argv )
{
 struct ExecBase *SysBase = *(struct ExecBase**)0x4; 
 struct Library *tst = OpenLibrary( (STRPTR)"example_lib.library",1);

 if( !tst )
 {
 	printf( "Global open fail, trying explicit local (1.3)\n" );
 	tst = OpenLibrary( (STRPTR)"progdir:example_lib.library",1);
	if( !tst )
	 printf( "Try the following: assign progdir: \"\"\n");
 }

 if( tst )
 {
 	CloseLibrary(tst);
	printf( "Opened Library, now trying to flush it\n");
	FlushLib( "example_lib.library" );
	
	if( FindName(&SysBase->LibList,(STRPTR)"example_lib.library") )
		printf("failed to flush\n");
	else	printf("Library flushed out\n");

 }
 else	printf( "failed to open Library\n");

 return 0;
}


void FlushLib( char *name)
{
    struct Library *result;
    struct ExecBase *SysBase = *(struct ExecBase**)0x4; 

    Forbid();
    if(result=(struct Library *)FindName(&SysBase->LibList,(STRPTR)name))
	RemLibrary(result);
    Permit();
}

