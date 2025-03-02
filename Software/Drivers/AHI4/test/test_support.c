#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/timer.h>

#include "amigus_ahi_sub.h"
#include "support.h"

#if defined (__VBCC__)
/* Don't care about ugly type issues with format strings! */
#pragma dontwarn 214
#endif

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/
struct AmiGUSBase * AmiGUSBase;

/* Taken over from lib_amigus.c */
/*
 defined in LIB:c.o:
struct ExecBase          * SysBase           = 0;
struct DosLibrary        * DOSBase           = 0;
struct IntuitionBase     * IntuitionBase     = 0;
struct Library           * UtilityBase       = 0;
struct Library           * ExpansionBase     = 0;
 */
struct Device            * TimerBase         = 0;

/******************************************************************************
 * Private functions / fields under test:
 *****************************************************************************/

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testSleep( VOID ) {

  BOOL failed = FALSE;
  int i;

#define NUM_CASES 5
  int cases[NUM_CASES][ 2 ] = {
    {   250, 2 },
    {  1500, 3 },
    {  5000, 4 },
    {  7500, 5 },
    { 10000, 6 }
  };

  for( i = 0; i < NUM_CASES; ++i ) {
    struct timeval before;
    struct timeval after;
    LONG delta_millis;
    BOOL tst;
    LONG millis_in = cases[i][0];
    LONG deviation = cases[i][1];

    printf( "Testing Sleep( %ld ) ...\n", millis_in );
    
    GetSysTime( &before );
    Sleep( millis_in );
    GetSysTime( &after );
    delta_millis = ((( after.tv_secs - before.tv_secs ) * 1000 )
                  + ( (LONG)after.tv_micro - (LONG)before.tv_micro ) / 1000 );
    tst = ( abs( millis_in - delta_millis ) <= deviation );
    printf( "Before %ld.%06ld - After %ld.%06ld - Delta %ldms \t %s\n",
            before.tv_secs,
            before.tv_micro,
            after.tv_secs,
            after.tv_micro,
            delta_millis,
            (tst) ? "passed" : "FAIL!!" );

    failed = !tst;
  }

  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  LONG error;
  BOOL failed = FALSE;
  struct AmiGUSBase * amiGUSBase;

  AmiGUSBase = malloc( sizeof( struct AmiGUSBase ) );
  memset( AmiGUSBase, 0, sizeof( struct AmiGUSBase ) );
  amiGUSBase = AmiGUSBase;

  if ( !amiGUSBase ) {

    printf( "Memory allocation 0 failed!\n" );
    return 20;
  }
  amiGUSBase->agb_TimerRequest = malloc( sizeof( struct IORequest ));
  if( !(amiGUSBase->agb_TimerRequest) ) {

    printf( "Memory allocation 1 failed!\n" );
    return 21;
  }
  error = OpenDevice( "timer.device", 0, amiGUSBase->agb_TimerRequest, 0 );
  if ( error ) {

    printf( "Opening timer.device failed!\n" );
    return 22;
  }
  amiGUSBase->agb_TimerBase = amiGUSBase->agb_TimerRequest->io_Device;
  TimerBase = amiGUSBase->agb_TimerRequest->io_Device;

  amiGUSBase->agb_LogFile = (BPTR)NULL;
  amiGUSBase->agb_LogMem = NULL;

  amiGUSBase->agb_DOSBase =
    (struct DosLibrary *) OpenLibrary( "dos.library", 37 );
  if( !(amiGUSBase->agb_DOSBase) ) {

    printf( "Opening dos.library failed!\n" );
    return 23;
  }
  DOSBase = amiGUSBase->agb_DOSBase;

  failed |= testSleep();

  if ( amiGUSBase->agb_LogFile ) {

    Close( amiGUSBase->agb_LogFile );
  }
  if( amiGUSBase->agb_DOSBase ) {

    CloseLibrary( (struct Library *) amiGUSBase->agb_DOSBase );
  }
  if( amiGUSBase->agb_TimerBase ) {

    CloseDevice( amiGUSBase->agb_TimerRequest );
  }
  if( amiGUSBase->agb_TimerRequest ) {

    free( amiGUSBase->agb_TimerRequest );
  }
  free( amiGUSBase );

  return ( failed ) ? 15 : 0;
}
