/*
 * This file is part of the mhiAmiGUS.library.
 *
 * mhiAmiGUS.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <exec/types.h>
#include <libraries/mhi.h>
#include <libraries/configvars.h>
#include <proto/exec.h>


#include "amigus_mhi.h"
/*
#include "amigus_codec.h"
#include "amigus_hardware.h"

#include "debug.h"
#include "errors.h"
#include "library.h"
#include "interrupt.h"
#include "support.h"
*/
#include "SDI_mhi_protos.h"

#define LOG_V(x)
#define LOG_D(x)

ASM( APTR ) SAVEDS MHIAllocDecoder(
  REG( a0, struct Task * task ),
  REG( d0, ULONG signal ),
  REG( a6, struct AmiGUS_MHI * base ) 
) {

  return NULL;
}

ASM( VOID ) SAVEDS MHIFreeDecoder(
  REG( a3, APTR handle ), 
  REG( a6, struct AmiGUS_MHI * base )
) {

  return;
}

ASM( BOOL ) SAVEDS MHIQueueBuffer(
  REG( a3, APTR handle ),
  REG( a0, APTR buffer ),
  REG( d0, ULONG size),
  REG( a6, struct AmiGUS_MHI * base )
) {

  return TRUE;
}

ASM( APTR ) SAVEDS MHIGetEmpty(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI * base )
) {

  return NULL;
}

ASM( UBYTE ) SAVEDS MHIGetStatus(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI * base )
) {

  return ( UBYTE ) 17;
}

ASM( VOID ) SAVEDS MHIPlay(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI * base )
) {

  return;
}

ASM( VOID ) SAVEDS MHIStop(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI * base )
) {

  return;
}

ASM( VOID ) SAVEDS MHIPause(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI * base )
) {

  return;
}

ASM( ULONG ) SAVEDS MHIQuery(
  REG( d1, ULONG query ),
  REG( a6, struct AmiGUS_MHI * base )
) {

  ULONG result = MHIF_UNSUPPORTED;

  LOG_D(( "D: MHIQuery start\n" ));
  switch ( query ) {
    case MHIQ_DECODER_NAME: {

      result = ( ULONG ) AMIGUS_MHI_DECODER;
      break;
    }
    case MHIQ_DECODER_VERSION: {

      result = ( ULONG ) AMIGUS_MHI_VERSION;
      break;
    }
    case MHIQ_AUTHOR: {

      result = ( ULONG ) AMIGUS_MHI_AUTHOR    " \n"        \
                         AMIGUS_MHI_COPYRIGHT " \n"        \
                         AMIGUS_MHI_ANNOTATION;
      break;
    }
    case MHIQ_CAPABILITIES: {

      result = ( ULONG )                                   \
               "audio/mpeg{audio/mp2,audio/mp3},"          \
               "audio/ogg{audio/vorbis},"                  \
               "audio/mp4{audio/aac},audio/aac,"           \
               "audio/flac";
      break;
    }
    case MHIQ_IS_HARDWARE:
    case MHIQ_MPEG1:
    case MHIQ_MPEG2:
    case MHIQ_MPEG25:
    case MHIQ_MPEG4:
    case MHIQ_LAYER1:
    case MHIQ_LAYER2:
    case MHIQ_LAYER3:
    case MHIQ_VARIABLE_BITRATE:
    case MHIQ_JOINT_STEREO:
    case MHIQ_BASS_CONTROL:
    case MHIQ_TREBLE_CONTROL:
    case MHIQ_MID_CONTROL:
    case MHIQ_5_BAND_EQ: {

      result = MHIF_SUPPORTED;
      break;
    }
    case MHIQ_IS_68K:
    case MHIQ_IS_PPC:
    case MHIQ_PREFACTOR_CONTROL:
    case MHIQ_10_BAND_EQ:
    case MHIQ_VOLUME_CONTROL:  // TODO
    case MHIQ_PANNING_CONTROL: // TODO
    case MHIQ_CROSSMIXING:     // TODO
    default: {
      break;
    }
  }
  LOG_V(( "V: Query %ld, result %ld = 0x%08lx \n", query, result, result ));
  LOG_D(( "D: MHIQuery done\n" ));
  return result;
}

ASM( VOID ) SAVEDS MHISetParam(
  REG( a3, APTR handle ),
  REG( d0, UWORD param ),
  REG( d1, ULONG value ),
  REG( a6, struct AmiGUS_MHI * base )
) {

  return;
}
