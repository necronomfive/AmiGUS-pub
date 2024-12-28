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

#include <exec/interrupts.h>

#include <hardware/intbits.h>

#include <proto/exec.h>

#include "amigus_private.h"
#include "debug.h"
#include "interrupt.h"

// TRUE = failure
BOOL CreateInterruptHandler( VOID ) {

  if (AmiGUSBase->agb_Interrupt) {

    LOG_D(("D: INT server in use!\n"));
    return FALSE;
  }

  LOG_D(("D: Creating INT server\n"));
  Disable();

  AmiGUSBase->agb_Interrupt = (struct Interrupt *)
      AllocMem(
          sizeof(struct Interrupt),
          MEMF_CLEAR | MEMF_PUBLIC
      );
  if ( AmiGUSBase->agb_Interrupt ) {

    AmiGUSBase->agb_Interrupt->is_Node.ln_Pri = 100;
    AmiGUSBase->agb_Interrupt->is_Node.ln_Name = "AMIGUS_AHI_INT";
    AmiGUSBase->agb_Interrupt->is_Data = AmiGUSBase;
    AmiGUSBase->agb_Interrupt->is_Code = (void (* )())handleInterrupt;

    AddIntServer(INTB_PORTS, AmiGUSBase->agb_Interrupt);

    Enable();

    LOG_D(("D: Created INT server\n"));
    return FALSE;
  }

  Enable();
  LOG_D(("D: Failed creating INT server\n"));
  // TODO: Display error?
  return TRUE;
}

VOID DestroyInterruptHandler( VOID ) {

  if ( !AmiGUSBase->agb_Interrupt ) {

    LOG_D(("D: No INT server to destroy!\n"));
    return;
  }
  
  LOG_D(("D: Destroying INT server\n"));

  Disable();
  RemIntServer( INTB_PORTS, AmiGUSBase->agb_Interrupt );
  Enable();

  FreeMem( AmiGUSBase->agb_Interrupt, sizeof(struct Interrupt) );
  AmiGUSBase->agb_Interrupt = NULL;

  LOG_D(("D: Destroyed INT server\n"));
}

ASM(LONG) /* __entry for vbcc ? */ SAVEDS INTERRUPT handleInterrupt (
  REG(a1, struct AmiGUSBasePrivate * amiGUSBase)
) {

  ULONG *current;
  BOOL canSwap;
  LONG reminder;          /* Read-back remaining FIFO samples in BYTES       */
  LONG target;            /* Target amount of BYTEs to fill into FIFO        */
  LONG copied;            /* Sum of BYTEs actually filled into FIFO this run */
  LONG minHwSampleSize;   /* Size of a single (mono / stereo) sample in BYTEs*/

  UWORD status = ReadReg16( AmiGUSBase->agb_CardBase,
                            AMIGUS_PCM_MAIN_INT_CONTROL );
  if ( !( status & ( AMIGUS_INT_F_PLAY_FIFO_EMPTY
                   | AMIGUS_INT_F_PLAY_FIFO_WATERMARK ) ) ) {
    
    return 0;
  }

  reminder = ReadReg16( AmiGUSBase->agb_CardBase,
                        AMIGUS_PCM_PLAY_FIFO_USAGE ) << 1;
  minHwSampleSize = AmiGUSSampleSizes[ AmiGUSBase->agb_HwSampleFormat ];

  /* Now find out target size to copy into FIFO during this interrupt run    */
  target = AMIGUS_PCM_PLAY_FIFO_BYTES;             /* all counting in BYTEs! */
  target -= reminder;                               /* deduct remaining FIFO */
  target -= minHwSampleSize;   /* and provide headroom for ALL sample sizes! */

  current = &( AmiGUSBase->agb_currentBuffer );
  canSwap = TRUE;
  copied = 0;
  
  while ( copied < target ) {

    if ( AmiGUSBase->agb_BufferIndex[ *current ] 
         < AmiGUSBase->agb_BufferMax[ *current ] ) {

      copied += (* AmiGUSBase->agb_CopyFunction)(
        AmiGUSBase->agb_Buffer[ *current ],
        &( AmiGUSBase->agb_BufferIndex[ *current ] ));

    } else if ( canSwap ) {

      *current ^= 0x00000001;
      canSwap = FALSE;

    } else {

      // Playback buffers empty, but FIFO could take more.
      // Not so good, Al...
      break;
    }
  }
  if ( status & AMIGUS_INT_F_PLAY_FIFO_EMPTY ) {

    /*
     Recovery from buffer underruns is a bit tricky.
     DMA will stay disabled until worker task prepared some buffers and 
     triggered a full playback init cycle to make us run again.
     */
    AmiGUSBase->agb_StateFlags |= AMIGUS_AHI_F_PLAY_UNDERRUN;
  }
  WriteReg16( AmiGUSBase->agb_CardBase,
              AMIGUS_PCM_PLAY_FIFO_WATERMARK,
              AmiGUSBase->agb_watermark );
  /* Clear AmiGUS control flags here!!! */
  WriteReg16( AmiGUSBase->agb_CardBase,
              AMIGUS_PCM_MAIN_INT_CONTROL,
              AMIGUS_INT_F_MASK_CLEAR
            | AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK );
  /* Signal sub task */
  if ( AmiGUSBase->agb_WorkerReady ) {

    Signal( (struct Task *) AmiGUSBase->agb_WorkerProcess,
            1 << AmiGUSBase->agb_WorkerWorkSignal );

  } else {

    // TODO: How do we handle worker not ready here? Maybe crying?
  }
  LOG_INT(( "INT: t %4ld c %4ld wm %4ld wr %ld\n",
            target,
            copied,
            AmiGUSBase->agb_watermark,
            AmiGUSBase->agb_WorkerReady ));
  return 1;
}
