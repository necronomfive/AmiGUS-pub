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

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/intuition.h>
#include <proto/utility.h>

#include <hardware/intbits.h>

#include <limits.h>

#include "amigus_private.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "worker.h"

UWORD ReadReg16(APTR amiGUS, ULONG offset) {

  return *((UWORD *)((ULONG) amiGUS + offset));
}

void WriteReg16(APTR amiGUS, ULONG offset, UWORD value) {

  *((UWORD *)((ULONG) amiGUS + offset)) = value;
}

ULONG ReadReg32(APTR amiGUS, ULONG offset) {

  return *((ULONG *)((ULONG) amiGUS + offset));
}

void WriteReg32(APTR amiGUS, ULONG offset, ULONG value) {

  *((ULONG *)((ULONG) amiGUS + offset)) = value;
}



/*
 * ---------------------------------------------------
 * Data definitions
 * ---------------------------------------------------
 */

/*
 * Array of supported sample rates, shall always be in 
 * sync with 
 * - AMIGUS_SAMPLE_RATE_* in amigus_hardware.h and
 * - AMIGUS_AHI_NUM_SAMPLE_RATES in amigus_public.h.
 */
const LONG AmiGUSSampleRates[ AMIGUS_AHI_NUM_SAMPLE_RATES ] = {

   8000, // AMIGUS_SAMPLE_RATE_8000  @ index 0x0000
  11025, // AMIGUS_SAMPLE_RATE_11025 @ index 0x0001
  16000, // AMIGUS_SAMPLE_RATE_16000 @ index 0x0002
  22050, // AMIGUS_SAMPLE_RATE_22050 @ index 0x0003
  24000, // AMIGUS_SAMPLE_RATE_24000 @ index 0x0004
  32000, // AMIGUS_SAMPLE_RATE_32000 @ index 0x0005
  44100, // AMIGUS_SAMPLE_RATE_44100 @ index 0x0006
  48000, // AMIGUS_SAMPLE_RATE_48000 @ index 0x0007
  96000  // AMIGUS_SAMPLE_RATE_96000 @ index 0x0008
};

const STRPTR AmiGUSOutputs[ AMIGUS_AHI_NUM_OUTPUTS ] = {

  "Line Out"
};

const STRPTR AmiGUSInputs[ AMIGUS_AHI_NUM_INPUTS ] = {

  "Line In",
  "Paula",
  "CD-ROM",
  "Decoder"
};

/*
 * Array of sample sizes in BYTE per hardware sample format.
 */
const UWORD AmiGUSSampleSizes[ AMIGUS_SAMPLE_FORMAT_STEREO_24BIT + 1 ] = {

  1, // AMIGUS_SAMPLE_FORMAT_MONO_8BIT    @ index 0x0000
  2, // AMIGUS_SAMPLE_FORMAT_STEREO_8BIT  @ index 0x0001
  2, // AMIGUS_SAMPLE_FORMAT_MONO_16BIT   @ index 0x0002
  4, // AMIGUS_SAMPLE_FORMAT_STEREO_16BIT @ index 0x0003
  3, // AMIGUS_SAMPLE_FORMAT_MONO_24BIT   @ index 0x0004
  6  // AMIGUS_SAMPLE_FORMAT_STEREO_24BIT @ index 0x0005
};

LONG FindAmiGUS(struct AmiGUSBasePrivate *amiGUSBase) {

  struct ConfigDev *configDevice = 0;
  ULONG serial = 0;
  UBYTE minute = 0;
  UBYTE hour   = 0;
  UBYTE day    = 0;
  UBYTE month  = 0;
  UWORD year   = 0;

  configDevice = FindConfigDev( configDevice,
                                AMIGUS_MANUFACTURER_ID,
                                AMIGUS_MAIN_PRODUCT_ID );
  if ( !configDevice ) {

    LOG_E(("E: AmiGUS not found\n"));
    return EAmiGUSNotFound;
  }
  if (   ( AMIGUS_MANUFACTURER_ID != configDevice->cd_Rom.er_Manufacturer )
      || ( AMIGUS_MAIN_PRODUCT_ID != configDevice->cd_Rom.er_Product ) 
     ) {

    LOG_E(("E: AmiGUS detection failed\n"));
    return EAmiGUSDetectError;
  }

  serial = configDevice->cd_Rom.er_SerialNumber;
  if ( AMIGUS_AHI_FIRMWARE_MINIMUM > serial ) {

    LOG_E(("E: AmiGUS firmware expected %08lx, actual %08lx\n",
           AMIGUS_AHI_FIRMWARE_MINIMUM, serial));
    return EAmiGUSFirmwareOutdated;
  }

  LOG_V(("V: AmiGUS firmware %08lx\n", serial));
  minute = (UBYTE)((serial & 0x0000003Ful)      );
  hour   = (UBYTE)((serial & 0x000007C0ul) >>  6);
  day    = (UBYTE)((serial & 0x0000F800ul) >> 11);
  month  = (UBYTE)((serial & 0x000F0000ul) >> 16);
  year   = (UWORD)((serial & 0xFFF00000ul) >> 20);
  LOG_I(("I: AmiGUS firmware date %04ld-%02ld-%02ld, %02ld:%02ld\n",
         year, month, day, hour, minute));

  amiGUSBase->agb_CardBase = (struct AmiGUS *)configDevice->cd_BoardAddr;
  LOG_I(("I: AmiGUS found at 0x%08lx\n", amiGUSBase->agb_CardBase));
  LOG_V(("V: AmiGUS address stored at 0x%08lx\n", &(amiGUSBase->agb_CardBase)));
  amiGUSBase->agb_UsageCounter = 0;

  return ENoError;
}

LONG FindSampleRateIdForValue( LONG sampleRate ) {

  LONG prev;
  LONG next = LONG_MAX;
  LONG i = 0;

  while ( AMIGUS_AHI_NUM_SAMPLE_RATES > i ) {

    prev = next;
    next = AmiGUSSampleRates[ i ] - sampleRate;

    /* We need absolute difference only. */
    if (0 > next) {
      next = -next;
    }

    LOG_D(("D: Frequency diff was %ld, next %ld\n", prev, next));
    if ( prev < next ) {
      /* Since frequencies are ordered, stop when difference is increasing. */
      break;
    }
    ++i;
  }
  LOG_I(("I: Using %ldHz aka ID %ld = 0x%02lx for requested %ldHz\n", 
         AmiGUSSampleRates[ i - 1 ],
         i - 1,
         i - 1,
         sampleRate));
  return i - 1;
}

LONG FindSampleRateValueForId( LONG id ) {

  LONG result = -1;
  if (( 0 <= id) &&
      ( AMIGUS_AHI_NUM_SAMPLE_RATES > id)) {

    result = AmiGUSSampleRates[ id ];
  }
  LOG_D(("D: Using %ldHz for ID %ld = 0x%02lx\n", result, id, id));
  return result;
}

VOID initAmiGUS( VOID ) {

  ULONG i;
  // Working maybe:
//  ULONG prefillSize = AMIGUS_PLAYBACK_FIFO_LONGS;
  ULONG prefillSize = 12; // 6000; /* in LONGs */
  APTR amiGUS = AmiGUSBase->agb_CardBase;
  LOG_D(("D: Init AmiGUS @ 0x%08lx\n", amiGUS));

  WriteReg16( amiGUS,
              AMIGUS_MAIN_SAMPLE_RATE,
              AMIGUS_SAMPLE_RATE_FLAG_DISABLE );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_FIFO_RESET,
              AMIGUS_FIFO_RESET );
  // Idea: Watermark is here 6 words aka 3 longs,
  //       2 24bit samples, ... 
  // Cool, always a fit for all sample widths.
  WriteReg16( amiGUS,
              AMIGUS_MAIN_FIFO_WATERMARK,
              // Watermark is WORDs, so using LONG value means half
              prefillSize );
  WriteReg16( amiGUS, 
              AMIGUS_MAIN_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_FLAG_MASK_CLEAR
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_FULL
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_FLAG_MASK_CLEAR
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_FULL
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_INT_ENABLE,
              /* Now set some! */
              AMIGUS_INT_FLAG_MASK_SET
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK );
  
  // now write twice the amount of data into FIFO to kick off playback
  for ( i = prefillSize; 0 < i; --i ) {

    WriteReg32( amiGUS,
                AMIGUS_MAIN_FIFO_WRITE,
                0L );
  }
  WriteReg16( amiGUS,
              AMIGUS_MAIN_SAMPLE_FORMAT,
              AmiGUSBase->agb_HwSampleFormat );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_SAMPLE_RATE,
              AmiGUSBase->agb_HwSampleRateId
            | AMIGUS_SAMPLE_RATE_FLAG_INTERPOLATION
            | AMIGUS_SAMPLE_RATE_FLAG_ENABLE );
  AmiGUSBase->agb_State = 1;
}

void stopAmiGUS(void) {

  APTR amiGUS = AmiGUSBase->agb_CardBase;
  LOG_D(("D: Stop AmiGUS @ 0x%08lx\n", amiGUS));

  WriteReg16( amiGUS, 
              AMIGUS_MAIN_SAMPLE_RATE, 
              AMIGUS_SAMPLE_RATE_FLAG_DISABLE );
  WriteReg16( amiGUS, 
              AMIGUS_MAIN_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_FULL
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_FULL
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_FIFO_RESET,
              0x0000 );
  AmiGUSBase->agb_State = 0;
}

// TRUE = failure
BOOL CreatePlaybackBuffers( VOID ) {

  LONG longSize;

  if ( AmiGUSBase->agb_Buffer[0] ) {

    LOG_D(("D: Playback buffers already exist!\n"));
    return FALSE;
  }

  /* BufferSize is in BYTEs! */
  AmiGUSBase->agb_BufferSize = AmiGUSBase->agb_AudioCtrl->ahiac_BuffSize;
  LOG_D(( "D: Allocating playback buffers a %ld BYTEs\n",
          AmiGUSBase->agb_BufferSize ));
  AmiGUSBase->agb_Buffer[0] = (ULONG *)
      AllocMem( AmiGUSBase->agb_BufferSize, MEMF_FAST | MEMF_CLEAR );
  if ( !AmiGUSBase->agb_Buffer[0] ) {

    LOG_E(("E: Could not allocate FAST RAM for buffer 0!\n"));
    return TRUE;
  }
  AmiGUSBase->agb_Buffer[1] = (ULONG *)
      AllocMem( AmiGUSBase->agb_BufferSize, MEMF_FAST | MEMF_CLEAR );
  if ( !AmiGUSBase->agb_Buffer[1] ) {

    LOG_E(("E: Could not allocate FAST RAM for buffer 1!\n"));
    return TRUE;
  }

  /* Buffers are ticking in LONGs! */
  longSize = AmiGUSBase->agb_BufferSize >> 2;

  AmiGUSBase->agb_BufferMax[ 0 ] = longSize;
  AmiGUSBase->agb_BufferMax[ 1 ] = longSize;
    /* All buffers are created empty - back to initial state! */
  AmiGUSBase->agb_BufferIndex[ 0 ] = longSize;
  AmiGUSBase->agb_BufferIndex[ 1 ] = longSize;
  AmiGUSBase->agb_currentBuffer = 0;

  LOG_I(( "I: Mix / copy up to %ld WORDs from AHI per pass\n",
          longSize << 1 ));

  LOG_D(("D: All playback buffers created\n"));
  return FALSE;
}

VOID DestroyPlaybackBuffers(VOID) {

  if ( AmiGUSBase->agb_Buffer[0] ) {

    FreeMem( AmiGUSBase->agb_Buffer[0], AmiGUSBase->agb_BufferSize );
    AmiGUSBase->agb_Buffer[0] = NULL;
    AmiGUSBase->agb_BufferIndex[0] = 0;
    LOG_D(("D: Free`ed buffer 0!\n"));
  }
  if ( AmiGUSBase->agb_Buffer[1] ) {

    FreeMem( AmiGUSBase->agb_Buffer[1], AmiGUSBase->agb_BufferSize );
    AmiGUSBase->agb_Buffer[1] = NULL;
    AmiGUSBase->agb_BufferIndex[1] = 0;
    LOG_D(("D: Free`ed buffer 1!\n"));
  }
  AmiGUSBase->agb_BufferSize = 0;
  LOG_D(("D: All playback buffers free`ed\n"));
}

// TRUE = failure
BOOL CreateInterruptHandler(VOID) {

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

VOID DestroyInterruptHandler(VOID) {

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

///#def ine INT_DEBUGGING
ASM(LONG) /*__entry for vbcc*/ SAVEDS INTERRUPT handleInterrupt (
  REG(a1, struct AmiGUSBasePrivate * amiGUSBase)
) {

  ULONG *current;
  BOOL canSwap;
  LONG reminder;
  LONG desired;

  UWORD status = ReadReg16( AmiGUSBase->agb_CardBase,
                            AMIGUS_MAIN_INT_CONTROL );
  if ( !( status & ( AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
                   | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK ) ) ) {
    
    return 0;
  }

#ifdef INT_DEBUGGING
  /*
   * Clear AmiGUS control flags here - 
   * Hope I can haz LOGz now!!!
   */
  if ( status & AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY ) {
    /* ... and yes, this causes buffer underruns. */
    WriteReg16( AmiGUSBase->agb_CardBase,
                AMIGUS_MAIN_INT_ENABLE,
                AMIGUS_INT_FLAG_MASK_CLEAR
              | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY );
  }
  WriteReg16( AmiGUSBase->agb_CardBase,
              AMIGUS_MAIN_INT_CONTROL,
              AMIGUS_INT_FLAG_MASK_CLEAR
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK );
#endif

  current = &( AmiGUSBase->agb_currentBuffer );  
  canSwap = TRUE;
  reminder = ReadReg16( AmiGUSBase->agb_CardBase,
                        AMIGUS_MAIN_FIFO_USAGE ) << 1; /* in BYTEs */
  // was: desired = AmiGUSBase->agb_watermark << 2; /* x2 and in BYTES*/
  desired = AMIGUS_PLAYBACK_FIFO_BYTES;
  desired -= reminder;
  desired -= AmiGUSSampleSizes[ AmiGUSBase->agb_HwSampleFormat ];

#ifdef INT_DEBUGGING
  LOG_D(("D: CB%01ld\n", *current));
#endif
  
  /* desired and buffers now counting BYTEs! */
  for ( ; ; ) {

    if ( !desired ) {
#ifdef INT_DEBUGGING
      LOG_D(("D: Fff\n"));
#endif
      break;
    }
    if ( AmiGUSBase->agb_BufferIndex[ *current ] < AmiGUSBase->agb_BufferMax[ *current ] ) {

#if 1
      desired -= (* AmiGUSBase->agb_CopyFunction)(
        AmiGUSBase->agb_Buffer[ *current ],
        &( AmiGUSBase->agb_BufferIndex[ *current ] ));
#else
      /* Old Version, needs 16bit, stereo */
      ULONG sampleAddress = (
          (( ULONG ) AmiGUSBase->agb_Buffer[ *current ])
           + (AmiGUSBase->agb_BufferIndex[ *current ] << 2));
      WriteReg32( AmiGUSBase->agb_CardBase,
                  AMIGUS_MAIN_FIFO_WRITE,
                  *(( ULONG * ) sampleAddress) );
      ++AmiGUSBase->agb_BufferIndex[ *current ];
      desired -= 4;
#endif

      continue;
    }
    if ( canSwap ) {

#ifdef INT_DEBUGGING
      LOG_D(("D: BS\n"));
#endif
      *current ^= 0x00000001;
      canSwap = FALSE;
      continue;
    }
#ifdef INT_DEBUGGING
    LOG_D(("D: Fmi\n"));
#endif
    break;
  }
  WriteReg16( AmiGUSBase->agb_CardBase,
              AMIGUS_MAIN_FIFO_WATERMARK,
              AmiGUSBase->agb_watermark );
  /* Clear AmiGUS control flags here!!! */
  WriteReg16( AmiGUSBase->agb_CardBase,
              AMIGUS_MAIN_INT_CONTROL,
              AMIGUS_INT_FLAG_MASK_CLEAR
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK );
  /* Signal sub task */
  if ( AmiGUSBase->agb_WorkerReady ) {

    Signal( (struct Task *) AmiGUSBase->agb_WorkerProcess,
            1 << AmiGUSBase->agb_WorkerWorkSignal );

  } else {
#ifdef INT_DEBUGGING
    LOG_D(("D: SNR\n"));
#endif
  }
  return 1;
}
