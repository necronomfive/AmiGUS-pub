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

#include <limits.h>

#include "amigus_private.h"
#include "debug.h"
#include "errors.h"
#include "samplerate.h"
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
  switch ( aAttribute ) {
    case AHIDB_Bits: {

      result = GetTagData( AHIDB_Bits, aDefault, aTagList );
      break;
    }
    case AHIDB_MaxChannels: {

      result = 1;
      break;
    }
    case AHIDB_Frequencies: {

      result = AMIGUS_PCM_SAMPLE_RATE_COUNT;
      break;
    }
    case AHIDB_Frequency: {

      result = FindSampleRateValueForId( aArgument );
      break;
    }
    case AHIDB_Index: {

      result = FindSampleRateIdForValue( aArgument );
      break;
    }
    case AHIDB_Author: {

      result = (LONG) AMIGUS_AHI_AUTHOR;
      break;
    }
    case AHIDB_Copyright: {

      result = (LONG) AMIGUS_AHI_COPYRIGHT;
      break;
    }
    case AHIDB_Version: {

      result = (LONG) AMIGUS_AHI_VERSION;
      break;
    }
    case AHIDB_Annotation: {

      result = (LONG) AMIGUS_AHI_ANNOTATION;
      break;
    }
    case AHIDB_Record: {

      result = GetTagData( AHIDB_Record, aDefault, aTagList );
      break;
    }
    case AHIDB_FullDuplex: {
      
      result = GetTagData( AHIDB_FullDuplex, aDefault, aTagList );
      break;
    }
    case AHIDB_Realtime: {
      
      result = GetTagData(AHIDB_Realtime, TRUE, aTagList);
      break;
    }
    case AHIDB_MaxPlaySamples: {

      ULONG bits = GetTagData(AHIDB_Bits, 0, aTagList);
      ULONG bytesPerSample = bits >> 3;
      ULONG flags = aAudioCtrl->ahiac_Flags;
      ULONG stereo = AHISF_KNOWSTEREO & flags;

      LOG_V(( "AHIDB_MaxPlaySamples "
              "bits %ld flags %lx stereo %lx bytes/sample %ld\n",
              bits,
              flags,
              stereo,
              bytesPerSample ));

      result = UDivMod32( AMIGUS_PCM_PLAY_FIFO_BYTES, bytesPerSample );
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
    case AHIDB_MaxRecordSamples: {

      ULONG bits = GetTagData(AHIDB_Bits, 0, aTagList);
      ULONG bytesPerSample = bits >> 3;
      ULONG flags = aAudioCtrl->ahiac_Flags;
      ULONG stereo = AHISF_KNOWSTEREO & flags;

      LOG_V(( "AHIDB_MaxRecordSamples "
              "bits %ld flags %lx stereo %lx bytes/sample %ld\n",
              bits,
              flags,
              stereo,
              bytesPerSample ));

      result = UDivMod32( AMIGUS_PCM_REC_FIFO_BYTES, bytesPerSample );

      if ( stereo  ) {

        result >>= 1;
      }
      break;
    }
    case AHIDB_MinMonitorVolume:
    case AHIDB_MaxMonitorVolume: {

      break;
    }
    case AHIDB_MinInputGain:
    case AHIDB_MinOutputVolume: {
      result = 0;
      break;
    }
    case AHIDB_MaxInputGain:
    case AHIDB_MaxOutputVolume: {
      result = USHRT_MAX + 1;
      break;
    }
    case AHIDB_Inputs:
      {
      result = AMIGUS_INPUTS_COUNT;
      break;
      }
    case AHIDB_Input:
      {
      result = ( LONG ) AmiGUSInputs[ aArgument ];
      break;
      }
    case AHIDB_Outputs:
      {
      result = AMIGUS_OUTPUTS_COUNT;
      break;
      }
    case AHIDB_Output:
      {
      result = ( LONG ) AmiGUSOutputs[ aArgument ];
      break;
      }
    case AHIDB_PingPong: {

      /* Suspicion: Asks if we can play samples reversed!? */
      LOG_V(("V: PingPong\n"));
      break;
    }
    default: {

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

Fixed getAhiVolumeFromAmiGUS( ULONG volumeRegisterOffset ) {

  ULONG volume = ReadReg16( AmiGUSBase->agb_CardBase, volumeRegisterOffset );
  if ( 0 != volume ) {

    ++volume;
  }
  return (Fixed) volume;
}

VOID setAhiVolumeToAmiGUS( Fixed volume, ULONG volumeRegisterOffset ) {

  if ( 0 != volume ) {

    --volume;
  }
  volume &= 0x0000FFff;
  volume |= volume << 16;
  WriteReg32( AmiGUSBase->agb_CardBase, volumeRegisterOffset, volume );
}

ASM(LONG) SAVEDS AMIGA_INTERRUPT AHIsub_HardwareControl(
  REG(a6, struct Library* aBase),
  REG(d0, ULONG aAttribute),
  REG(d1, LONG aArgument),
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {

  LONG result = FALSE;
  if ( !AmiGUSBase->agb_CardBase ) {

    /* No card - not supported! */
    LOG_W(( "W: AHIsub_HardwareControl - No card found!\n" ));
    return result;
  }
  switch ( aAttribute ) {
    case AHIC_OutputVolume_Query: {

      result =
        (LONG) getAhiVolumeFromAmiGUS( AMIGUS_PCM_PLAY_VOLUME_LEFT );
      break;
    }
    case AHIC_OutputVolume: {
      
      setAhiVolumeToAmiGUS( aArgument, AMIGUS_PCM_PLAY_VOLUME );
      result = TRUE;
      break;
    }
    case AHIC_MonitorVolume:
    case AHIC_MonitorVolume_Query:
    case AHIC_InputGain_Query: {

      result =
        (LONG) getAhiVolumeFromAmiGUS( AMIGUS_PCM_REC_VOLUME_LEFT );
      break;
    }
    case AHIC_InputGain: {
      
      setAhiVolumeToAmiGUS( aArgument, AMIGUS_PCM_REC_VOLUME );
      result = TRUE;
      break;
    }
    case AHIC_Input:
    case AHIC_Input_Query:
    case AHIC_Output:
    case AHIC_Output_Query: {

      break;
    }
    default: {

      DisplayError( EHardwareControlNotImplemented );
      break;
    }
  }
  LOG_D(( "D: AHIsub_HardwareControl %ld %ld => %ld\n",
          aAttribute - AHI_TagBase,
          aArgument,
          result ));
  return result;
}
