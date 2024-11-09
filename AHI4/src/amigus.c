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

#include "amigus.h"
#include "amigus_private.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "worker.h"

#if defined(__VBCC__)

__reg("d1") ULONG __GET_REG_D1()="\t";
#define GET_REG(reg) __GET_ ## reg()

#elif defined(__SASC)

#define GET_REG getreg

#define getreg __builtin_getreg
extern long getreg(int);

#define REG_D1 1

#endif

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


/* Basic functions */

ASM(ULONG) SAVEDS AHIsub_AllocAudio(
//  REG(a6, struct Library* aBase) works fine with vbcc, but not with SASC,
  REG(a1, struct TagItem* aTagList),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
  struct TagItem *stateTag = 0;
  struct TagItem *tag = 0;
  ULONG frequencyId = 0;
  ULONG frequency = 0;
  ULONG samplesAmiGus = 0;
  ULONG samplesAhi = 0;
  ULONG reminder = 0;
  UBYTE isStereo = FALSE;
  UBYTE isHifi = FALSE;
  UBYTE isRealtime = FALSE;
  UBYTE bitsPerAmiGusSample = 0;

  /* Will rely on AHI provided mixing and timing */
  ULONG result = AHISF_MIXING | AHISF_TIMING;

  LOG_D(("D: AHIsub_AllocAudio start\n"));
  LogTicks(0x03);

  /*
   * ------------------------------------------------------
   * Part 1: Allocate this AmiGUS to exactly this callee,
   *         re-entrance/interrupt/multitasking safe.
   * ------------------------------------------------------
   */
  Disable();
  if ( AmiGUSBase->agb_UsageCounter ) {

    Enable();
    DisplayError( EDriverInUse );
    return AHISF_ERROR;
  
  } else {

    ++AmiGUSBase->agb_UsageCounter;
    Enable();
  }
  LOG_D(("D: Alloc`ed AmiGUS AHI hardware\n"));

  /*
   * ------------------------------------------------------
   * Part 2: Extract audio mode information
   * ------------------------------------------------------
   */
  /* Find nearest supported frequency and provide it back */
  frequencyId = FindNearestFrequencyIndex( aAudioCtrl->ahiac_MixFreq );
  frequency = Frequencies[ frequencyId ];
  LOG_V(("D: Using %ldHz = 0x%02lx for requested %ldHz\n",
         frequency,
         frequencyId,
         aAudioCtrl->ahiac_MixFreq));
  aAudioCtrl->ahiac_MixFreq = frequency;

  /* Parse aTagList */
  stateTag = aTagList;
  while ( tag = NextTagItem( &stateTag ) ) {

    switch ( tag->ti_Tag ) {
      case AHIDB_Bits: {
        bitsPerAmiGusSample = (UBYTE)tag->ti_Data;
        break;
      }
      case AHIDB_Stereo: {
        isStereo = (UBYTE)tag->ti_Data;
        result |= AHISF_KNOWSTEREO;
        break;
      }
      case AHIDB_HiFi: {
        isHifi = (UBYTE)tag->ti_Data;
        LOG_D(("D: TODO: Why is HiFi in the list?\n"));
//        result |= AHISF_KNOWHIFI;
        break;
      }
      case AHIDB_Realtime: {
        isRealtime = (UBYTE)tag->ti_Data;
        break;
      }
      default: {
        break;
      }
    }
  }

  LOG_D(("D: Mode is %ldbit, %ld stereo, %ld HiFi, %ld Realtime, %ldHz\n",
         bitsPerAmiGusSample, isStereo, isHifi, isRealtime, frequency
       ));
  if (( isHifi ) || ( !isStereo ) || ( 16 != bitsPerAmiGusSample )) {
    DisplayError( EAudioModeNotImplemented );
    return AHISF_ERROR;
  }

  /*
   * ------------------------------------------------------
   * Part 3: Apply information to AmiGUS & driver.
   * ------------------------------------------------------
   */

  // TODO: Initialize AmiGUS with that information.

  // TODO: switch copy functions here for HiFi modes
  //       Others will be plain LONG copys.

  /*
   * ------------------------------------------------------
   * Part 4: Prepare slave task communication.
   * ------------------------------------------------------
   */
  AmiGUSBase->agb_WorkerWorkSignal = -1;
  AmiGUSBase->agb_WorkerStopSignal = -1;
  AmiGUSBase->agb_MainProcess = ( struct Process * ) FindTask( NULL );
  AmiGUSBase->agb_MainSignal = AllocSignal( -1 );
  if ( -1 == AmiGUSBase->agb_MainSignal ) {

    DisplayError( EMainProcessSignalsFailed );
    return AHISF_ERROR;
  }
  AmiGUSBase->agb_WorkerReady = FALSE;

  LOG_D(("D: AHIsub_AllocAudio done, expected %ld actual %ld\n", 
         AHISF_MIXING | AHISF_TIMING | AHISF_KNOWSTEREO,
         result));

  return result;
}

ASM(void) SAVEDS AHIsub_FreeAudio(
  REG(a6, struct Library* aBase),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(("D: AHIsub_FreeAudio start\n"));

  /*
   * ------------------------------------------------------
   * Part 4: Free slave task communication.
   * ------------------------------------------------------
   */
  /* Freeing a non-alloc`ed signal, i.e. -1, is harmless */
  FreeSignal( AmiGUSBase->agb_MainSignal );
  AmiGUSBase->agb_MainSignal = -1;
  LOG_D(("D: Free`ed main signal\n"));

  /*
   * ------------------------------------------------------
   * Part 1: De-allocate this AmiGUS,
   *         re-entrance/interrupt/multitasking safe.
   * ------------------------------------------------------
   */
  Disable();
  if ( !AmiGUSBase->agb_UsageCounter ) {

    Enable();
    DisplayError( EDriverNotInUse );
    return;
  
  } else {

    --AmiGUSBase->agb_UsageCounter;
    Enable();
  }
  LOG_D(("D: Free`ed AmiGUS AHI hardware\n"));

  LOG_D(("D: AHIsub_FreeAudio done\n"));

  return;
}

ASM(void) SAVEDS AHIsub_Disable(
  REG(a6, struct Library* aBase),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
/*
  LOG_D(("AHIsub_Disable\n"));
  WriteReg16( AmiGUSBase->agb_CardBase,
              AMIGUS_MAIN_INT_ENABLE,
              AMIGUS_INT_FLAG_MASK_CLEAR
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK
            );
*/
  Disable();
  return;
}


ASM(void) SAVEDS AHIsub_Enable(
  REG(a6, struct Library* aBase),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
/*
  LOG_D(("AHIsub_Enable\n"));
  WriteReg16( AmiGUSBase->agb_CardBase,
              AMIGUS_MAIN_INT_ENABLE,
              AMIGUS_INT_FLAG_MASK_SET
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK
            );
         */
  Enable();
  return;
}

ASM(ULONG) SAVEDS AHIsub_Start(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(("D: AHIsub_Start start\n"));

  AHIsub_Update( aBase, aFlags, aAudioCtrl );

  if ( AHISF_PLAY & aFlags ) {

    LOG_D(("D: Creating playback buffers\n", (LONG) AmiGUSBase));
    if ( CreatePlaybackBuffers() ) {

      LOG_D(("D: No playback buffers, failed.\n"));
      return AHIE_UNKNOWN;
    }
  }

  LOG_D(("D: Creating worker process for AmiGUSBase @ %08lx\n", (LONG) AmiGUSBase));
  if ( CreateWorkerProcess() ) {

    LOG_D(("D: No worker, failed.\n"));
    return AHIE_UNKNOWN;
  }
  if ( CreateInterruptHandler() ) {
  
    LOG_D(("D: No INT handler, failed.\n"));
    return AHIE_UNKNOWN;
  }

  initAmiGUS();
  LOG_D(("D: AHIsub_Start done\n"));
  return AHIE_OK;
}

ASM(void) SAVEDS AMIGA_INTERRUPT AHIsub_Update(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(("D: AHIsub_Update start\n"));
  LOG_V(("V: old ctrl 0x%08lx, new ctrl 0x%08lx, longSize %ld, BuffSamples %lu, Min %lu, Max %lu, BuffSize %lu, BuffType %lu\n",
          AmiGUSBase->agb_AudioCtrl,
          aAudioCtrl,
          AmiGUSBase->agb_BufferSize,
          aAudioCtrl->ahiac_BuffSamples,
          aAudioCtrl->ahiac_MinBuffSamples,
          aAudioCtrl->ahiac_MaxBuffSamples,
          aAudioCtrl->ahiac_BuffSize,
          aAudioCtrl->ahiac_BuffType
       ));
  AmiGUSBase->agb_AudioCtrl = aAudioCtrl;
  LOG_D(("D: AHIsub_Update stop\n"));
  return;
}

ASM(void) SAVEDS AHIsub_Stop(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aFlags),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(("D: AHIsub_Stop start\n"));

  LOG_D(("D: Read FIFO level %04lx\n",
         ReadReg16(
             AmiGUSBase->agb_CardBase,
             AMIGUS_MAIN_FIFO_USAGE) ));
  stopAmiGUS();

  DestroyInterruptHandler();
  DestroyWorkerProcess();
  DestroyPlaybackBuffers();


  LOG_D(("AHIsub_Stop done\n"));
  return;
}

/* Acceleration functions */

ASM(ULONG) SAVEDS AHIsub_SetVol(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aChannel),
  REG(d1, Fixed aVolume),
  REG(d2, sposition aPan),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl),
  REG(d3, ULONG aFlags)
) {
//  LOG_D(("AHIsub_SetVol\n"));
  return AHIS_UNKNOWN;
}

ASM(ULONG) SAVEDS AHIsub_SetFreq(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aChannel),
  REG(d1, ULONG aFreq),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl),
  REG(d2, ULONG aFlags)
) {
//  LOG_D(("AHIsub_SetFreq\n"));
  return AHIS_UNKNOWN;
}

ASM(ULONG) SAVEDS AHIsub_SetSound(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aChannel),
  REG(d1, UWORD aSound),
  REG(d2, ULONG aOffset),
  REG(d3, LONG aLength),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl),
  REG(d4, ULONG aFlags)
) {
//  LOG_D(("AHIsub_SetSound\n"));
  return AHIS_UNKNOWN;
}

ASM(ULONG) SAVEDS AHIsub_SetEffect(
  REG(a6, struct Library* aBase),
  REG(a0, APTR aEffect),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
  LOG_D(("AHIsub_SetEffect\n"));
  return 0L;
}

ASM(ULONG) SAVEDS AHIsub_LoadSound(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aSound),
  REG(d1, ULONG aType),
  REG(a0, APTR aInfo),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
//  LOG_D(("AHIsub_LoadSound\n"));
  return AHIS_UNKNOWN;
}

ASM(ULONG) SAVEDS AHIsub_UnloadSound(
  REG(a6, struct Library* aBase),
  REG(d0, UWORD aSound),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
//  LOG_D(("AHIsub_UnloadSound\n"));
  return AHIS_UNKNOWN;
}

/* Query functions */

/*
 * ---------------------------------------------------
 * Data definitions
 * ---------------------------------------------------
 */

/*
 * Array of supported frequencies, shall always be in 
 * sync with AMIGUS_AHI_NUM_FREQUENCIES from amigus_public.h
 */
const LONG Frequencies[ AMIGUS_AHI_NUM_FREQUENCIES ] =
  {
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

const STRPTR Outputs[ AMIGUS_AHI_NUM_OUTPUTS ] =
  {
  "Line Out"
  };

const STRPTR Inputs[ AMIGUS_AHI_NUM_INPUTS ] =
  {
  "Line In",
  "Paula",
  "CD-ROM",
  "Decoder"
  };

ASM(LONG) SAVEDS AHIsub_GetAttr(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aAttribute),
  REG(d1, LONG aArgument),
  REG(d2, LONG aDefault),
  REG(a1, struct TagItem* aTagList),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {
  LONG result = aDefault;
  switch ( aAttribute )
    {
    case AHIDB_Bits:
      {
      result = GetTagData(AHIDB_Bits, aDefault, aTagList);
      break;
      }
    case AHIDB_MaxChannels:
      {
      result = 1;
      break;
      }
    case AHIDB_Frequencies:
      {
      result = AMIGUS_AHI_NUM_FREQUENCIES;
      break;
      }
    case AHIDB_Frequency:
      {
      result = Frequencies[ aArgument ];
      break;
      }
    case AHIDB_Index:
      {
      result = FindNearestFrequencyIndex(aArgument);
      break;
      }
    case AHIDB_Author:
      {
      result = (LONG) AMIGUS_AHI_AUTHOR;
      break;
      }
    case AHIDB_Copyright:
      {
      result = (LONG) AMIGUS_AHI_COPYRIGHT;
      break;
      }
    case AHIDB_Version:
      {
      result = (LONG) AMIGUS_AHI_VERSION;
      break;
      }
    case AHIDB_Annotation:
      {
      result = (LONG) AMIGUS_AHI_ANNOTATION;
      break;
      }
    case AHIDB_Record:
      {
      result = AMIGUS_AHI_RECORD;
      break;
      }
    case AHIDB_FullDuplex:
      {
      result = AMIGUS_AHI_FULL_DUPLEX;
      break;
      }
    case AHIDB_Realtime:
      {
      result = GetTagData(AHIDB_Realtime, TRUE, aTagList);
      break;
      }
    case AHIDB_MaxPlaySamples:
      {
      ULONG bits = GetTagData(AHIDB_Bits, 0, aTagList);
      ULONG bytesPerSample = bits / 8;
      ULONG flags = aAudioCtrl->ahiac_Flags;
      ULONG stereo = AHISF_KNOWSTEREO & flags;
      
      LOG_V(("AHIDB_MaxPlaySamples bits %ld flags %lx stereo %lx bytes/sample %ld\n", 
            bits, 
            flags,
            stereo,
            bytesPerSample));

      result = UDivMod32(AMIGUS_PLAYBACK_FIFO_BYTES, bytesPerSample);
      /*
      Could get reminder by getRegD1(); like
        __reg("d1") ULONG __getRegD1()="\t";
        #define getRegD1() __getRegD1()
      */
      if ( stereo  ) {
        result >>= 1;
      }
      break;
      }
    case AHIDB_MaxRecordSamples:
      {
      /* TODO: Ask how much record buffer we have!? */
      break;
      }
    case AHIDB_MinInputGain:
    case AHIDB_MinMonitorVolume:
    case AHIDB_MinOutputVolume:
      {
//      result = 0;
      break;
      }
    case AHIDB_MaxInputGain:
    case AHIDB_MaxMonitorVolume:
    case AHIDB_MaxOutputVolume:
      {
//      result = USHRT_MAX;
      break;
      }
    case AHIDB_Inputs:
      {
      result = AMIGUS_AHI_NUM_INPUTS;
      break;
      }
    case AHIDB_Input:
      {
      result = ( LONG ) Inputs[ aArgument ];
      break;
      }
    case AHIDB_Outputs:
      {
      result = AMIGUS_AHI_NUM_OUTPUTS;
      break;
      }
    case AHIDB_Output:
      {
      result = ( LONG ) Outputs[ aArgument ];
      break;
      }
    case AHIDB_PingPong:
      {
      LOG_V(("V: PingPong\n"));
      break;
      }
    default:
      {
      DisplayError( EGetAttrNotImplemented );
      break;
      }
    }

  LOG_D(("D: AHIsub_GetAttr %ld %ld %ld => %ld\n", 
        aAttribute - AHI_TagBase,
        aDefault,
        aArgument,
        result));
  return result;
}

/* Mixer functions */

ASM(LONG) SAVEDS AMIGA_INTERRUPT AHIsub_HardwareControl(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aAttribute),
  REG(d1, LONG aArgument),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {
  LOG_D(("AHIsub_HardwareControl\n"));
  return 0L;
}

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

LONG FindNearestFrequencyIndex(LONG aFrequency) {

  LONG prev;
  LONG next = LONG_MAX;
  LONG i = 0;

  while ( AMIGUS_AHI_NUM_FREQUENCIES > i ) {

    prev = next;
    next = Frequencies[ i ] - aFrequency;

    /* We need absolute difference only. */
    if (0 > next) {
      next = -next;
    }

    LOG_V(("V: Frequency diff was %ld, next %ld\n", prev, next));
    if ( prev < next ) {
      /* Since frequencies are ordered, stop when difference is increasing. */
      break;
    }
    ++i;
  }
  return i - 1;
}

LONG FindNearestFrequency(LONG aFrequency) {

  LONG index = FindNearestFrequencyIndex( aFrequency );
  LONG result = Frequencies[ index ];
  LOG_V(("V: Using %ldHz for requested %ldHz\n", result, aFrequency));

  return result;
}

void initAmiGUS(void) {

  ULONG i;
  // Working maybe:
  ULONG prefillSize = AMIGUS_PLAYBACK_FIFO_LONGS;
  //ULONG prefillSize = 6; /* in LONGs */ 
  APTR amiGUS = AmiGUSBase->agb_CardBase;
  LOG_D(("D: Init AmiGUS @ 0x%08lx\n", amiGUS));

  WriteReg16( amiGUS,
              AMIGUS_MAIN_SAMPLE_RATE,
              AMIGUS_SAMPLE_RATE_FLAG_DISABLE
            );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_FIFO_RESET,
              AMIGUS_FIFO_RESET);
  // Idea: Watermark is here 6 words aka 3 longs,
  //       2 24bit samples, ... 
  // Cool, always a fit for all sample widths.
  WriteReg16( amiGUS,
              AMIGUS_MAIN_FIFO_WATERMARK,
              prefillSize >> 1
            );
  WriteReg16( amiGUS, 
              AMIGUS_MAIN_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_FLAG_MASK_CLEAR
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_FULL
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK
            );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_FLAG_MASK_CLEAR
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_FULL
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK
            );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_INT_ENABLE,
              /* Now set some! */
              AMIGUS_INT_FLAG_MASK_SET
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK
            );
  
  // now write twice the amount of data into FIFO to kick off playback
  for ( i = prefillSize; 0 < i; --i ) {

    WriteReg32( amiGUS,
                AMIGUS_MAIN_FIFO_WRITE,
                0L );
  }
  WriteReg16( amiGUS,
              AMIGUS_MAIN_SAMPLE_FORMAT,
              AMIGUS_SAMPLE_FORMAT_STEREO_16BIT
            );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_SAMPLE_RATE,
              AMIGUS_SAMPLE_RATE_44100
            | AMIGUS_SAMPLE_RATE_FLAG_INTERPOLATION
            | AMIGUS_SAMPLE_RATE_FLAG_ENABLE
            );
  AmiGUSBase->agb_State = 1;
}

void stopAmiGUS(void) {

  APTR amiGUS = AmiGUSBase->agb_CardBase;
  LOG_D(("D: Stop AmiGUS @ 0x%08lx\n", amiGUS));

  WriteReg16( amiGUS, 
              AMIGUS_MAIN_SAMPLE_RATE, 
              AMIGUS_SAMPLE_RATE_FLAG_DISABLE
            );
  WriteReg16( amiGUS, 
              AMIGUS_MAIN_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_FULL
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK
            );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_FLAG_PLAYBACK_FIFO_EMPTY
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_FULL
            | AMIGUS_INT_FLAG_PLAYBACK_FIFO_WATERMARK
            );
  WriteReg16( amiGUS,
              AMIGUS_MAIN_FIFO_RESET,
              0x0000
            );
  AmiGUSBase->agb_State = 0;
}

// TRUE = failure
BOOL CreatePlaybackBuffers(VOID) {

  /* Equals words here for stereo 16bit and we use that here! */
  ULONG samplesPerPass = AmiGUSBase->agb_AudioCtrl->ahiac_BuffSamples;
  ULONG longsPerPass = samplesPerPass;

  if ( AmiGUSBase->agb_Buffer[0] ) {

    LOG_D(("D: Playback buffers already exist!\n"));
    return FALSE;
  }

  AmiGUSBase->agb_BufferSize = AmiGUSBase->agb_AudioCtrl->ahiac_BuffSize >> 2;
  LOG_D(("D: Allocating playback buffers a %ld LONGs\n", AmiGUSBase->agb_BufferSize ));
  AmiGUSBase->agb_Buffer[0] = (ULONG *)
      AllocMem( AmiGUSBase->agb_BufferSize << 2, MEMF_FAST | MEMF_CLEAR );
  if ( !AmiGUSBase->agb_Buffer[0] ) {

    LOG_D(("Could not allocate FAST RAM for buffer 0!\n"));
    return TRUE;
  }
  AmiGUSBase->agb_Buffer[1] = (ULONG *)
      AllocMem( AmiGUSBase->agb_BufferSize << 2, MEMF_FAST | MEMF_CLEAR );
  if ( !AmiGUSBase->agb_Buffer[1] ) {

    LOG_D(("Could not allocate FAST RAM for buffer 1!\n"));
    return TRUE;
  }
  /*
   * Buffers definitely are empty here,
   * all these have to tick in LONGs!
   */
  AmiGUSBase->agb_BufferIndex[0] = longsPerPass;
  AmiGUSBase->agb_BufferIndex[1] = longsPerPass;
  AmiGUSBase->agb_BufferMax[0] = longsPerPass;
  AmiGUSBase->agb_BufferMax[1] = longsPerPass;
/*
  if ( AmiGUSBase->agb_BufferSize > (AMIGUS_PLAYBACK_FIFO_WORDS >> 1) ) {

    AmiGUSBase->agb_watermark = AMIGUS_PLAYBACK_FIFO_WORDS >> 1;

  } else {

    AmiGUSBase->agb_watermark = AmiGUSBase->agb_BufferSize >> 1;
  }
*/
  AmiGUSBase->agb_watermark = longsPerPass << 1;
  LOG_D(("D: Mix %ld samples = %ld LONGs per pass, watermark %ld WORDs\n",
         samplesPerPass,
         longsPerPass,
         AmiGUSBase->agb_watermark));
  AmiGUSBase->agb_currentBuffer = 0;

  LOG_D(("D: All playback buffers created\n"));
  return FALSE;
}

VOID DestroyPlaybackBuffers(VOID) {

  if ( AmiGUSBase->agb_Buffer[0] ) {

    FreeMem( AmiGUSBase->agb_Buffer[0],
             AmiGUSBase->agb_BufferSize << 2 );
    AmiGUSBase->agb_Buffer[0] = NULL;
    AmiGUSBase->agb_BufferIndex[0] = 0;
    LOG_D(("D: Free`ed buffer 0!\n"));
  }
  if ( AmiGUSBase->agb_Buffer[1] ) {

    FreeMem( AmiGUSBase->agb_Buffer[1],
             AmiGUSBase->agb_BufferSize << 2 );
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
  desired = ( AmiGUSBase->agb_watermark << 2 ); /* x2 and in BYTEs */
  desired -= reminder;

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
    if ( AmiGUSBase->agb_BufferIndex[ *current ] < AmiGUSBase->agb_BufferMax[ *current] ) {

      ULONG sampleAddress = (
          (( ULONG ) AmiGUSBase->agb_Buffer[ *current ])
           + (AmiGUSBase->agb_BufferIndex[ *current ] << 2));
      WriteReg32( AmiGUSBase->agb_CardBase,
                  AMIGUS_MAIN_FIFO_WRITE,
                  *(( ULONG * ) sampleAddress) );
      ++AmiGUSBase->agb_BufferIndex[ *current ];
      desired -= 4;

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
