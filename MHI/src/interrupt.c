/*
 * This file is part of the mhiamigus.library.
 *
 * mhiamigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiamigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiamigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <exec/types.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <libraries/mhi.h>

#include <proto/exec.h>

#include "amigus_mhi.h"
#include "amigus_hardware.h"
#include "amigus_vs1063.h"
#include "debug.h"
#include "errors.h"
#include "interrupt.h"
#include "support.h"

/******************************************************************************
 * Interrupt functions - private functions.
 *****************************************************************************/

/**
 * Interrupt handler function, checking the status of all AmiGUS cards
 * found so far and filling their buffers accordingly.
 *
 * @param base Pointer to the driver library's base address.
 *
 * @return 1 if there was at least one card's interrupt pending that was handled,
 *         0 otherwise.
 */
ASM( LONG ) /* __entry for vbcc ? */ SAVEDS INTERRUPT handleInterrupt (
  REG( a1, struct AmiGUS_MHI_Base * base )) {

  struct MinList * clients = &( AmiGUS_MHI_Base->agb_Clients );
  struct AmiGUS_MHI_Handle * handle;
  LONG result = 0;

  FOR_LIST( clients, handle, struct AmiGUS_MHI_Handle * ) {

    APTR card = handle->agch_CardBase;
    const UWORD enable = ReadReg16( card, AMIGUS_CODEC_INT_ENABLE );
    const UWORD control = ReadReg16( card, AMIGUS_CODEC_INT_CONTROL );
    const UWORD status = enable & control;

    /*
    // This is super-spammy - but tells you what is wrong if int is stalling!
    LOG_INT(( "INT: h 0x%08lx c 0x%08lx s 0x%04lx\n",
              handle, card, status ));
    // */
    if ( status & ( AMIGUS_CODEC_INT_F_FIFO_EMPTY
                  | AMIGUS_CODEC_INT_F_FIFO_WATERMRK )) {

      if ( MHIF_PLAYING == handle->agch_Status ) {
  
        FillCodecBuffer( handle );
      }

      /* Clear AmiGUS control flags here!!! */
      WriteReg16( card,
                  AMIGUS_CODEC_INT_CONTROL,
                  AMIGUS_INT_F_CLEAR
                  | AMIGUS_CODEC_INT_F_FIFO_EMPTY
                  | AMIGUS_CODEC_INT_F_FIFO_WATERMRK );
      result = 1;
    }
  }

  return result;
}

/******************************************************************************
 * Interrupt functions - public function definitions.
 *****************************************************************************/

LONG CreateInterruptHandler( VOID ) {

  if ( AmiGUS_MHI_Base->agb_Interrupt ) {

    LOG_D(( "D: INT server in use!\n" ));
    return ENoError;
  }

  LOG_D(( "D: Creating INT server\n" ));
  Disable();

  AmiGUS_MHI_Base->agb_Interrupt = ( struct Interrupt * )
      AllocMem(
          sizeof( struct Interrupt ),
          MEMF_CLEAR | MEMF_PUBLIC
      );
  if ( AmiGUS_MHI_Base->agb_Interrupt ) {

    AmiGUS_MHI_Base->agb_Interrupt->is_Node.ln_Pri = 100;
    AmiGUS_MHI_Base->agb_Interrupt->is_Node.ln_Name = "AmiGUS_MHI_Base_INT";
    AmiGUS_MHI_Base->agb_Interrupt->is_Data = ( APTR ) AmiGUS_MHI_Base;
    AmiGUS_MHI_Base->agb_Interrupt->is_Code = ( VOID ( * )( )) handleInterrupt;

    AddIntServer( INTB_PORTS, AmiGUS_MHI_Base->agb_Interrupt );

    Enable();

    LOG_D(( "D: Created INT server\n" ));
    return ENoError;
  }

  Enable();
  LOG_D(( "D: Failed creating INT server\n" ));

  return EAllocateInterrupt;
}

VOID DestroyInterruptHandler( VOID ) {

  if ( !AmiGUS_MHI_Base->agb_Interrupt ) {

    LOG_D(("D: No INT server to destroy!\n"));
    return;
  }
  
  LOG_D(("D: Destroying INT server\n"));

  Disable();
  RemIntServer( INTB_PORTS, AmiGUS_MHI_Base->agb_Interrupt );
  Enable();

  FreeMem( AmiGUS_MHI_Base->agb_Interrupt, sizeof( struct Interrupt ) );
  AmiGUS_MHI_Base->agb_Interrupt = NULL;

  LOG_D(("D: Destroyed INT server\n"));
}

VOID FillCodecBuffer( struct AmiGUS_MHI_Handle * handle ) {

  APTR card = handle->agch_CardBase;
  struct AmiGUS_MHI_Buffer * current = handle->agch_CurrentBuffer;
  const APTR tail = ( const APTR ) &( handle->agch_Buffers.mlh_Tail );
  /* Read-back remaining FIFO samples in BYTES */
  LONG reminder = ReadReg16( card, AMIGUS_CODEC_FIFO_USAGE ) << 1;
  /**********************************************************
   * Keep some 1 LONG = 4 BYTE distance to FIFO's limit---+ *
   * Bytes currently remaining in CODEC FIFO------+       | *
   * Size of the CODEC FIFO---+                   |       | *
   *                          V                   V       V */
  LONG target = AMIGUS_CODEC_PLAY_FIFO_BYTES - reminder - 4;
  LONG copied = 0;

  if ( !current ) {

    LOG_INT(( "INT: No current buffer!\n" ));
    return;
  }
  while ( copied < target ) {

    if ( current->agmb_BufferIndex < current->agmb_BufferMax ) {

      ULONG data = current->agmb_Buffer[ current->agmb_BufferIndex ];
      WriteReg32( card, AMIGUS_CODEC_FIFO_WRITE, data );
      // LOG_INT(( "INT: 0x%08lx\n", data ));
      ++current->agmb_BufferIndex;
      copied += 4;

    } else if ( current->agmb_BufferExtraBytes ) {

      ULONG data = 0;
      BYTE shift = 24;
      ULONG address = ( ULONG ) current->agmb_Buffer;
      address += current->agmb_BufferIndex << 2;
      copied += current->agmb_BufferExtraBytes;
      
      while ( current->agmb_BufferExtraBytes ) {

        UBYTE * extraData = ( UBYTE * ) address;
        UBYTE byteData = *extraData;

        data |= byteData << shift;

        ++address;
        --current->agmb_BufferExtraBytes;
        shift -= 8;
      }
      data |= ( GetVS1063EndFill( card ) & (0xFFffFFff >> ( 24 - shift )));
      LOG_INT(( "INT: ed 0x%08lx\n", data ));
      WriteReg32( card, AMIGUS_CODEC_FIFO_WRITE, data );
      copied += 4;

    } else if ( tail != ( const APTR ) current->agmb_Node.mln_Succ ) {

      LOG_INT(( "INT: ob 0x%08lx i %ld m %ld\n",
                current,
                current->agmb_BufferIndex,
                current->agmb_BufferMax ));
      Signal( handle->agch_Task, handle->agch_Signal );
      current = ( struct AmiGUS_MHI_Buffer * ) current->agmb_Node.mln_Succ;
      handle->agch_CurrentBuffer = current;
      LOG_INT(( "INT: nb 0x%08lx i %ld m %ld\n",
                current,
                current->agmb_BufferIndex,
                current->agmb_BufferMax ));

    } else {

      // Playback buffers empty, but FIFO could take more. - End of Stream?
      handle->agch_Status = MHIF_OUT_OF_DATA;
      Signal( handle->agch_Task, handle->agch_Signal );
      LOG_INT(( "INT: lb 0x%08lx i %ld m %ld\n",
                current,
                current->agmb_BufferIndex,
                current->agmb_BufferMax ));
      break;
    }
  }
  LOG_INT(( "INT: Playback r %4ld t %4ld c %4ld cb 0x%08lx nb 0x%08lx\n",
            reminder,
            target,
            copied,
            current,
            current->agmb_Node.mln_Succ ));
}
