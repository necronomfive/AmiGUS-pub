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

#include <dos/dostags.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>

#include "amigus_ahi_sub.h"
#include "amigus_hardware.h"
#include "amigus_pcm.h"
#include "buffers.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "worker.h"

#if 0

VOID FillBuffer( BYTE buffer ) {

  ULONG i = 0;
  const ULONG longSize = AmiGUSBase->agb_BufferMax[ buffer ];
  const ULONG halfLongSize = longSize >> 1;
  ULONG *longBuffer = (ULONG *)AmiGUSBase->agb_Buffer[ buffer ];

//  LOG_D(( "D: FB%1ld\n", buffer ));
  for ( ; halfLongSize > i; ++i) {
  
    longBuffer[ i ] = 0x00000000;
  }
  for ( ; longSize > i; ++i) {
  
    longBuffer[ i ] = 0x44444444;
  }
  AmiGUSBase->agb_BufferIndex[ buffer ] = 0;
  AmiGUSBase->agb_watermark = longSize << 1;
}

#endif
#if 0

VOID FillBuffer( BYTE buffer ) {

  ULONG i = 0;
  const ULONG longSize = AmiGUSBase->agb_BufferMax[ buffer ];
  ULONG *longBuffer = (ULONG *)AmiGUSBase->agb_Buffer[ buffer ];

//  LOG_D(( "D: FB%1ld -> %08lx\n", buffer, longBuffer ));
//  LOG_D(( "D: FB%1ld\n", buffer ));
  for ( ; longSize > i; ++i) {

    if ( i & 0x00000008 ) {
      longBuffer[ i ] = 0x44444444;
    } else {
      longBuffer[ i ] = 0x00000000;
    }
  }
  AmiGUSBase->agb_BufferIndex[ buffer ] = 0;
  AmiGUSBase->agb_watermark = longSize << 1;
}

#endif
#if 1

typedef ASM(BOOL) PreTimerType( REG(a2, struct AHIAudioCtrlDrv *) );
typedef ASM(VOID) PostTimerType( REG(a2, struct AHIAudioCtrlDrv *) );

INLINE VOID FillBuffer( BYTE buffer ) {

//  LOG_D(( "D: FB%1ld\n", buffer ));
  struct AmiGUSPcmPlayback * playback = &AmiGUSBase->agb_Playback;
  struct AHIAudioCtrlDrv *audioCtrl = AmiGUSBase->agb_AudioCtrl;
  PreTimerType *preTimer = (PreTimerType *)audioCtrl->ahiac_PreTimer;
  PostTimerType *postTimer = (PostTimerType *)audioCtrl->ahiac_PostTimer;

  /*
   * 1) Call user hook ahiac_PlayerFunc().
   */
  CallHookPkt( audioCtrl->ahiac_PlayerFunc, audioCtrl, NULL );

  /*
   * 2) Optionally call ahiac_PreTimerFunc().
   */
  if ( !( preTimer( audioCtrl ) ) ) {

    /*
     * 3) Call user hook ahiac_MixerFunc().
     */
    CallHookPkt( audioCtrl->ahiac_MixerFunc,
                 audioCtrl,
                 (APTR) playback->agpp_Buffer[ buffer ] );

    /*
     * 4) Convert and feed the buffer into the audio hardware.
     *    Either can be non cache-able (bad) or flush the caches (better).
     *    Well, it is a register... no caching here. :(
     *    The good news: the interrupt handler will take that responsibility.
     * ************************************************************************
     * Conversion happens during feeding to hardware, 
     * just need to remember how much we got here.
     *
     * "How the buffer will be filled is indicated by ahiac_Flags.
     * It is always filled with signed 16-bit 
     * (32 bit if AHIACB_HIFI in in ahiac_Flags is set) words, 
     * even if playback is 8 bit. 
     * If AHIDBB_STEREO is set (in ahiac_Flags), 
     * data for left and right channel are interleaved [...] "
     *
     * Remember: Buffers ticking in LONGs (hence the final shift) !!!
     */
    playback->agpp_BufferMax[ buffer ] =
      AlignByteSizeForSamples( audioCtrl->ahiac_BuffSamples ) >> 2;
    playback->agpp_BufferIndex[ buffer ] = 0; /* buffer full  */
  }
  /*
   * 5) Optionally call ahiac_PostTimerFunc().
   */
  postTimer( audioCtrl );
}

#endif

INLINE VOID HandlePlayback( VOID ) {

  struct AmiGUSPcmPlayback * playback = &AmiGUSBase->agb_Playback;

  ULONG i;
  ULONG k = playback->agpp_CurrentBuffer;
  for ( i = 0; 2 > i; ++i ) {
    
    if ( playback->agpp_BufferIndex[ k ] 
        >= playback->agpp_BufferMax[ k ] ) {

      FillBuffer( k );
    }
    k ^= 0x00000001;
  }
  if ( AMIGUS_AHI_F_PLAY_UNDERRUN & AmiGUSBase->agb_StateFlags ) {

    LOG_W(( "W: Recovering from playback buffer underrun.\n" ));
    /*
    We suffered a full underrun before...
    FIFO empty, both playback buffers empty...
    so the playback and interrupt are kind of dead.
    Just enabling playback again would un-align 24bit stereo playback,
    so instead we can go through full playback init and start cycle.
    Bonus: resets the playback state flags. :)
    */
    StartAmiGusPcmPlayback();
  }

  LOG_INT(( "WORKER: Playback i0 %5ld m0 %5ld i1 %5ld m1 %5ld\n",
            playback->agpp_BufferIndex[ 0 ],
            playback->agpp_BufferMax[ 0 ],
            playback->agpp_BufferIndex[ 1 ],
            playback->agpp_BufferMax[ 1 ] ));
}

INLINE VOID HandleRecording( VOID ) {

  struct AmiGUSPcmRecording * recording = &( AmiGUSBase->agb_Recording );
  struct AHIAudioCtrlDrv *audioCtrl = AmiGUSBase->agb_AudioCtrl;
  struct AHIRecordMessage *message = &( recording->agpr_RecordingMessage );

  ULONG i;
  ULONG k = recording->agpr_CurrentBuffer;
  for ( i = 0; 2 > i; ++i ) {
    
    if ( recording->agpr_BufferIndex[ k ] 
        >= recording->agpr_BufferMax[ k ] ) {

      LOG_INT(( "WORKER: Draining b%1ld b* 0x%08lx bl %5ld\n",
                k,
                recording->agpr_Buffer[ k ],
                recording->agpr_BufferIndex[ k ] ));
      LOG_INT(( "WORKER: LONGs   0 -   3: %08lx %08lx %08lx %08lx\n",
                recording->agpr_Buffer[ k ][ 0 ],
                recording->agpr_Buffer[ k ][ 1 ],
                recording->agpr_Buffer[ k ][ 2 ],
                recording->agpr_Buffer[ k ][ 3 ] ));
      LOG_INT(( "WORKER: LONGs 100 - 103: %08lx %08lx %08lx %08lx\n",
                recording->agpr_Buffer[ k ][ 100 ],
                recording->agpr_Buffer[ k ][ 101 ],
                recording->agpr_Buffer[ k ][ 102 ],
                recording->agpr_Buffer[ k ][ 103 ] ));

      message->ahirm_Buffer = ( APTR ) recording->agpr_Buffer[ k ];
      message->ahirm_Length = recording->agpr_BufferIndex[ k ];
      message->ahirm_Length <<= 2;                            // LONGs to BYTEs
      message->ahirm_Length >>= recording->agpr_AhiSampleShift;   // to samples

      LOG_INT(( "WORKER: Using CopyFuncId  %ld sending sample type %ld\n", 
                recording->agpr_CopyFunctionId,
                message->ahirm_Type ));

      CallHookPkt( audioCtrl->ahiac_SamplerFunc,
                   audioCtrl,
                   ( APTR ) message );

      recording->agpr_BufferIndex[ k ] = 0; /* buffer empty  */

    } else {

      LOG_INT(( "WORKER: Skipping b %ld\n", k ));
    }
    k ^= 0x00000001;
  }
  if ( AMIGUS_AHI_F_REC_OVERFLOW & AmiGUSBase->agb_StateFlags ) {

    LOG_W(( "W: Recovering from recording buffer overflow.\n" ));
    /*
    We suffered a full overflow before...
    FIFO full, both recording buffers full...
    so the recording and interrupt are kind of dead.
    Bonus: resets the recording state flags. :)
    */
    StartAmiGusPcmRecording();
  }

  LOG_INT(( "WORKER: Recording i0 %5ld m0 %5ld i1 %5ld m1 %5ld\n",
            recording->agpr_BufferIndex[ 0 ],
            recording->agpr_BufferMax[ 0 ],
            recording->agpr_BufferIndex[ 1 ],
            recording->agpr_BufferMax[ 1 ] ));
}

/*__entry for vbcc*/ SAVEDS VOID WorkerProcess( VOID ) {

  ULONG signals = TRUE;

  LOG_D(("D: Worker for AmiGUSBase @ %08lx starting...\n", (LONG) AmiGUSBase));

  AmiGUSBase->agb_WorkerWorkSignal = AllocSignal( -1 );
  AmiGUSBase->agb_WorkerStopSignal = AllocSignal( -1 );
  
  if ( ( -1 != AmiGUSBase->agb_WorkerWorkSignal )
    && ( -1 != AmiGUSBase->agb_WorkerStopSignal )) {

    /* Tell master worker is alive */
    Signal(
      (struct Task *) AmiGUSBase->agb_MainProcess,
      1 << AmiGUSBase->agb_MainSignal
    );
    // SetSignal(0, 1 << agb_WorkerStopSignal );
    while ( signals ) {

      if ( AMIGUS_AHI_F_PLAY_STARTED & AmiGUSBase->agb_StateFlags ) {

        HandlePlayback();
      }
      if ( AMIGUS_AHI_F_REC_STARTED & AmiGUSBase->agb_StateFlags ) {

        HandleRecording();
      }

      AmiGUSBase->agb_WorkerReady = TRUE;
      signals = Wait(
          SIGBREAKF_CTRL_C
        | ( 1 << AmiGUSBase->agb_WorkerWorkSignal )
        | ( 1 << AmiGUSBase->agb_WorkerStopSignal )
      );
      /* 
       All signals break the wait, 
       but only "work" continues the playback loop, 
       so the others are masked away.
       */
       signals &= ( 1 << AmiGUSBase->agb_WorkerWorkSignal );
    }
  } else {
    /* Well... */
    DisplayError( EWorkerProcessSignalsFailed );
  }
  LOG_D(("D: Worker for AmiGUSBase @ %08lx ending...\n", (LONG) AmiGUSBase));

  FreeSignal( AmiGUSBase->agb_WorkerWorkSignal );
  AmiGUSBase->agb_WorkerWorkSignal = -1;
  FreeSignal( AmiGUSBase->agb_WorkerStopSignal );
  AmiGUSBase->agb_WorkerStopSignal = -1;

  /* Stop multitasking here - master will resume it TODO: ??? */
  //  Forbid();

  Signal(
      (struct Task *) AmiGUSBase->agb_MainProcess,
      1 << AmiGUSBase->agb_MainSignal
  );
  AmiGUSBase->agb_WorkerProcess = NULL;
  AmiGUSBase->agb_WorkerReady = FALSE;
  LOG_D(("D: Worker for AmiGUSBase @ %08lx ended.\n", (LONG) AmiGUSBase));
}

/*
 * TRUE = failure
 */
BOOL CreateWorkerProcess( VOID ) {

  /* Prevent worker from waking up until we are ready. */
  Forbid();
  if ( AmiGUSBase->agb_WorkerProcess ) {

    Permit();
    LOG_D(("D: Worker already exists!\n"));
    return FALSE;
  }
  
  AmiGUSBase->agb_WorkerProcess =
      CreateNewProcTags( NP_Entry, (ULONG) &WorkerProcess,
                         NP_Name, (ULONG) LIBRARY_NAME,
                         NP_Priority, (ULONG) 127,
                         TAG_DONE, 0 
                       );
  if ( AmiGUSBase->agb_WorkerProcess ) {

    AmiGUSBase->agb_WorkerProcess->pr_Task.tc_UserData = AmiGUSBase;

  } /* Potential error handling later */

  /* Well... Worker can only wake up if we ... */
  Permit();

  if ( AmiGUSBase->agb_WorkerProcess ) {
    /* Wait for worker signalling main */
    Wait( 1 << AmiGUSBase->agb_MainSignal );
    /* Check if worker is alive or dead */
    if ( !AmiGUSBase->agb_WorkerProcess ) {
      /* Worker died meanwhile */
      DisplayError( EWorkerProcessDied );
      return TRUE;
    }
  } else {
    /* Handling worker never really been created. */
    DisplayError( EWorkerProcessCreationFailed );
    return TRUE;
  }
}

VOID DestroyWorkerProcess(VOID) {

  if ( !AmiGUSBase->agb_WorkerProcess ) {

    LOG_D(("D: No worker process to destroy!\n"));
    return;
  }

  LOG_D(("D: Destroying worker process\n"));

  if ( -1 != AmiGUSBase->agb_WorkerStopSignal ) {

    /* Kill the playback worker to stop the playback */
    Signal(( struct Task * ) AmiGUSBase->agb_WorkerProcess,
          1 << AmiGUSBase->agb_WorkerStopSignal );

    /* Wait for the worker to die and imply Permit() */
    Wait( 1 << AmiGUSBase->agb_MainSignal );
  }
  if ( !AmiGUSBase->agb_WorkerProcess ) {
    LOG_D(("D: Destroyed worker process\n"));
  }
}
