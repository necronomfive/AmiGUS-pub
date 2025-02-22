/*
 * This file is part of the mhiAmiGUS.library driver.
 *
 * mhiAmiGUS.library driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library driver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <libraries/mhi.h>

#include <proto/exec.h>

#include "amigus_mhi.h"
#include "amigus_hardware.h"
#include "debug.h"
#include "interrupt.h"

INLINE VOID HandlePlayback( VOID ) {

  LOG_INT(( "INT: Playback \n" ));
}

ASM(LONG) /* __entry for vbcc ? */ SAVEDS INTERRUPT handleInterrupt (
  REG(a1, struct AmiGUSBasePrivate * amiGUSBase)
) {
  const UWORD status = ReadReg16( AmiGUSBase->agb_CardBase,
                                  AMIGUS_CODEC_INT_CONTROL );
  if ( !( status & ( AMIGUS_INT_F_CODEC_FIFO_EMPTY
                   | AMIGUS_INT_F_CODEC_FIFO_WATERMRK )) ) {

    return 0;
  }

  if ( MHIF_PLAYING == AmiGUSBase->agb_ClientHandle.agch_Status ) {

    HandlePlayback();
/*
    if ( status & AMIGUS_INT_F_PLAY_FIFO_EMPTY ) {

      /*
       Recovery from buffer underruns is a bit tricky.
       DMA from FIFO to DAC will stay disabled until worker task prepared some
       buffers and triggered a full playback init cycle to make it run again.
      * /
      AmiGUSBase->agb_StateFlags |= AMIGUS_AHI_F_PLAY_UNDERRUN;
    }
*/
  }

  /* Clear AmiGUS control flags here!!! */
  WriteReg16( AmiGUSBase->agb_CardBase,
              AMIGUS_CODEC_INT_CONTROL,
              AMIGUS_INT_F_CLEAR
              | AMIGUS_INT_F_CODEC_FIFO_EMPTY
              | AMIGUS_INT_F_CODEC_FIFO_WATERMRK );

  return 1;
}

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
          sizeof( struct Interrupt ),
          MEMF_CLEAR | MEMF_PUBLIC
      );
  if ( AmiGUSBase->agb_Interrupt ) {

    AmiGUSBase->agb_Interrupt->is_Node.ln_Pri = 100;
    AmiGUSBase->agb_Interrupt->is_Node.ln_Name = "AMIGUS_MHI_INT";
    AmiGUSBase->agb_Interrupt->is_Data = AmiGUSBase;
    AmiGUSBase->agb_Interrupt->is_Code = (void (* )())handleInterrupt;

    AddIntServer( INTB_PORTS, AmiGUSBase->agb_Interrupt );

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

  FreeMem( AmiGUSBase->agb_Interrupt, sizeof( struct Interrupt ) );
  AmiGUSBase->agb_Interrupt = NULL;

  LOG_D(("D: Destroyed INT server\n"));
}
