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

#include <exec/libraries.h>
#include <proto/utility.h>

#include "amigus_private.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "SDI_AHI4_protos.h"

/* Query functions */

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
      result = AMIGUS_AHI_NUM_SAMPLE_RATES;
      break;
      }
    case AHIDB_Frequency:
      {
      result = FindSampleRateValueForId( aArgument );
      break;
      }
    case AHIDB_Index:
      {
      result = FindSampleRateIdForValue( aArgument );
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
      result = ( LONG ) AmiGUSInputs[ aArgument ];
      break;
      }
    case AHIDB_Outputs:
      {
      result = AMIGUS_AHI_NUM_OUTPUTS;
      break;
      }
    case AHIDB_Output:
      {
      result = ( LONG ) AmiGUSOutputs[ aArgument ];
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

  /* Not supported or unknown: */
  return FALSE;
}
