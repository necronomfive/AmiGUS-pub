/*
 * This file is part of the mhiamigus.library.
 *
 * mhiamigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiamigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiamigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#ifdef NULL
#undef NULL
#endif /* NULL */


#include <proto/exec.h>
#include <proto/timer.h>

#include <exec/types.h>
#include <devices/timer.h>

struct Library * ExecBase = NULL;
struct Device * TimerBase = NULL;
struct timerequest * TimerIORequest = NULL;

VOID Sleep( ULONG seconds, ULONG micros ) {

  struct timerequest sleepRequest = *TimerIORequest;
  sleepRequest.tr_node.io_Command = TR_ADDREQUEST;
  sleepRequest.tr_time.tv_secs = seconds;
  sleepRequest.tr_time.tv_micro = micros;
  SendIO(( struct IORequest * ) &sleepRequest );
  Wait( 1 << sleepRequest.tr_node.io_Message.mn_ReplyPort->mp_SigBit );
  WaitIO(( struct IORequest * ) &sleepRequest );
}

#if 0

#include <proto/alib.h>

// Goes by: 
// vc +kick13 test_sleep.c $VBCC/targets/m68k-kick13/lib/amiga.lib
#else
// And this just: 
// vc +kick13 test_sleep.c 

VOID NonConflictingNewMinList( struct MinList * list ) {

  list->mlh_Head = ( struct MinNode * ) &list->mlh_Tail;
  list->mlh_Tail = NULL;
  list->mlh_TailPred = ( struct MinNode * ) list;
}

struct MsgPort * CreatePort( BYTE * name, LONG priority ) {

  struct MsgPort * result;
  BYTE signal = AllocSignal( -1L );

  if ( -1 == signal ) {

    return NULL;
  }

  result = ( struct MsgPort * ) AllocMem( sizeof( struct MsgPort ),
                                          MEMF_PUBLIC | MEMF_CLEAR );
  if ( !result ) {

    FreeSignal( signal );
    return NULL;
  }
  result->mp_Node.ln_Name = name;
  result->mp_Node.ln_Pri  = priority;
  result->mp_Node.ln_Type = NT_MSGPORT;
  result->mp_Flags        = PA_SIGNAL;
  result->mp_SigBit       = signal;
  result->mp_SigTask      = ( struct Task * ) FindTask( 0 ); // Current task!
  if ( name ) {

    AddPort(result);

  } else {

    NonConflictingNewMinList(( struct MinList * )&( result->mp_MsgList ));
  }
  return result;
}

VOID DeletePort( struct MsgPort * port ) {

  if ( !port ) {

    return;
  }
  if ( port->mp_Node.ln_Name ) {

    RemPort(port);
  }
  if (( port->mp_Flags & PF_ACTION ) == PA_SIGNAL ) {

    FreeSignal( port->mp_SigBit );
    port->mp_Flags = PA_IGNORE;
  }
  FreeMem( port, sizeof( struct MsgPort ));
}

struct IORequest * CreateExtIO( struct MsgPort * port, ULONG ioSize ) {

  struct IORequest * result = NULL;
  if ( port ) {
    result = AllocMem( ioSize, MEMF_PUBLIC | MEMF_CLEAR );
	if ( result ) {
	    result->io_Message.mn_ReplyPort = port;
	    result->io_Message.mn_Length = ioSize;
	    result->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
	}
  }
  return( result );
}

VOID DeleteExtIO( struct IORequest * ioReq ) {

  if ( ioReq ) {

    ioReq->io_Message.mn_Node.ln_Succ = NULL;
    ioReq->io_Device = NULL;
    FreeMem( ioReq, ioReq->io_Message.mn_Length);
  }
}

#endif
/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  ExecBase = OpenLibrary( "exec.library", 34 );
  if ( !ExecBase ) {
    printf( "exec failed\n" );
    return 21;
  }

  struct MsgPort * timerPort;
  struct timerequest tickRequest;

  timerPort = CreatePort( NULL, 0 );
  if ( !timerPort ) {
    printf( "port failed\n" );
    return 22;
  }
  TimerIORequest = ( struct timerequest * )
    CreateExtIO( timerPort, sizeof( struct timerequest ));
  if ( !TimerIORequest ) {
    printf( "TimerIORequest failed\n" );
    return 22;
  }
  /* open the timer.device */
  BYTE error = OpenDevice( TIMERNAME,
                           UNIT_MICROHZ,
                           ( struct IORequest * ) TimerIORequest,
                           0 );
  if ( !error ) {
    /* Set the TimerBase to enable calling timer.device's functions */
    TimerBase = ( struct Device * ) TimerIORequest->tr_node.io_Device;

  } else {

    printf( "timer device failed\n" );
    return 23;
  }

  tickRequest = *TimerIORequest;
  tickRequest.tr_node.io_Command = TR_ADDREQUEST;

  for ( ULONG i = 0 ; i < 10 ; ++i ) {
    printf( "%lu\n", i);
    tickRequest.tr_time.tv_secs = 2;
    tickRequest.tr_time.tv_micro = 2;
#if 0
    DoIO((struct IORequest*)&tickRequest);
#elif 0
    SendIO((struct IORequest*)&tickRequest);
    Wait( 1 << tickRequest.tr_node.io_Message.mn_ReplyPort->mp_SigBit );
    WaitIO((struct IORequest*)&tickRequest);
#elif 1
    Sleep( 2, 2 );
#endif
  }
  printf( "done\n");

  if( TimerBase ) {
    CloseDevice(( struct IORequest * ) TimerIORequest );
    TimerBase = 0;
  }
  if ( TimerIORequest ) {
    DeleteExtIO(( struct IORequest * ) TimerIORequest );
    TimerIORequest = 0;
  }
  if ( timerPort) {
    DeletePort( timerPort );
    timerPort = NULL;
  }
  if ( ExecBase ) {
    CloseLibrary( ExecBase );
    ExecBase = NULL;
  }
  return 0;
}
