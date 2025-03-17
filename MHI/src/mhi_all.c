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

#include <libraries/mhi.h>
#include <libraries/configvars.h>
#include <proto/exec.h>

#include "amigus_codec.h"
#include "amigus_hardware.h"
#include "amigus_mhi.h"
#include "debug.h"
#include "errors.h"
#include "library.h"
#include "interrupt.h"
#include "support.h"
#include "SDI_mhi_protos.h"

VOID FlushAllBuffers( struct AmiGUS_MHI_Handle * clientHandle ) {

  struct List * buffers = ( struct List * )&clientHandle->agch_Buffers;
  struct Node * buffer;

  while ( buffer = RemHead( buffers )) {

    LOG_V(( "V: Removing / free'ing MHI buffer 0x%08lx, "
      "size %ld LONGs, index %ld\n",
      buffer,
      (( struct AmiGUS_MHI_Buffer * ) buffer)->agmb_BufferMax,
      (( struct AmiGUS_MHI_Buffer * ) buffer)->agmb_BufferIndex ));
    FreeMem( buffer, sizeof( struct AmiGUS_MHI_Buffer ) );
  }
  clientHandle->agch_CurrentBuffer = NULL;
  LOG_D(( "D: All buffers flushed.\n" ));
}

VOID UpdateEqualizer( UWORD bandLevel, UWORD percent ) {

  APTR card = AmiGUS_MHI_Base->agb_CardBase;
  // -32 .. +32 = ((( 0 .. 100 ) * 2655 ) / 4096 ) - 32
  // gain = percent * 2655 / 4096 - 32
  LONG intermediate = (( percent * 2655 ) >> 12 ) - 32;
  WORD gain = ( WORD ) intermediate;
  LOG_D(( "D: Calculated gain %ld = %ld for %ld%\n",
          intermediate, gain, percent ));

  UpdateVS1063Equalizer( card, bandLevel, gain );
}

ASM( APTR ) SAVEDS MHIAllocDecoder(
  REG( a0, struct Task * task ),
  REG( d0, ULONG signal ),
  REG( a6, struct AmiGUS_MHI_Base * base ) 
) {

  APTR result = NULL;
  ULONG error = ENoError;

  LOG_D(( "D: MHIAllocDecoder start\n" ));

  Forbid();
  if ( AmiGUS_MHI_Base->agb_ConfigDevice->cd_Driver ) {

    error = EAmiGUSInUseError;

  } else if ( AmiGUS_MHI_Base->agb_UsageCounter ) {

    error = EDriverInUse;

  } else {

    // Now we own the card!
    ++AmiGUS_MHI_Base->agb_UsageCounter;
    AmiGUS_MHI_Base->agb_ConfigDevice->cd_Driver = AmiGUS_MHI_Base;
  }
  Permit();

  if ( !error ) {
 
    APTR card = AmiGUS_MHI_Base->agb_CardBase;
    struct AmiGUS_MHI_Handle * handle = &AmiGUS_MHI_Base->agb_ClientHandle;
    handle->agch_Task = task;
    handle->agch_Signal = signal;
    handle->agch_Status = MHIF_STOPPED;
    handle->agch_CurrentBuffer = NULL;
    NonConflictingNewMinList( &handle->agch_Buffers );

    result = ( APTR ) handle;

    LOG_D(( "D: AmiGUS MHI accepted task 0x%08lx and signal 0x%08lx.\n",
            task, signal ));

    LOG_D(( "D: Initializing VS1063 codec\n" ));
    InitVS1063Codec( card );
    InitVS1063Equalizer( card, TRUE, AmiGUSDefaultEqualizer );
    CreateInterruptHandler();

  } else {

    // Takes care of the log entry, too. :)
    DisplayError( error );
  }
  
  LOG_D(( "D: MHIAllocDecoder done\n" ));
  return result;
}

ASM( VOID ) SAVEDS MHIFreeDecoder(
  REG( a3, APTR handle ), 
  REG( a6, struct AmiGUS_MHI_Base * base )
) {

  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  struct Task * task = clientHandle->agch_Task;
  ULONG signal = clientHandle->agch_Signal;
  ULONG error = ENoError;

  LOG_D(( "D: MHIFreeDecoder start for task 0x%08lx\n", task ));

  if (( &AmiGUS_MHI_Base->agb_ClientHandle != clientHandle ) ||
      ( !task )) {

      LOG_W(( "W: AmiGUS MHI does not know task 0x%08lx and signal 0x%08lx"
        " - hence not free'd.\n",
        task, signal ));
      LOG_D(( "D: MHIFreeDecoder done\n" ));
      return;
  }

  Forbid();
  if (( AmiGUS_MHI_Base->agb_UsageCounter ) &&
      ( AmiGUS_MHI_Base->agb_ConfigDevice->cd_Driver == AmiGUS_MHI_Base )) {

    clientHandle->agch_Task = NULL;
    clientHandle->agch_Signal = 0;
    --AmiGUS_MHI_Base->agb_UsageCounter;
    AmiGUS_MHI_Base->agb_ConfigDevice->cd_Driver = NULL;

  } else {

    error = EDriverNotInUse;
  }
  Permit();
  if ( error ) {

    DisplayError( error );

  } else {

    FlushAllBuffers( clientHandle );

    LOG_D(( "D: AmiGUS MHI free'd up task 0x%08lx and signal 0x%08lx.\n",
            task, signal ));
    DestroyInterruptHandler();
  }
  LOG_D(( "D: MHIFreeDecoder done\n" ));
  return;
}

ASM( BOOL ) SAVEDS MHIQueueBuffer(
  REG( a3, APTR handle ),
  REG( a0, APTR buffer ),
  REG( d0, ULONG size),
  REG( a6, struct AmiGUS_MHI_Base * base )
) {
  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  struct List * buffers = ( struct List * )&clientHandle->agch_Buffers;
  struct AmiGUS_MHI_Buffer * mhiBuffer;

  LOG_D(( "D: MHIQueueBuffer start\n" ));
  if ( &AmiGUS_MHI_Base->agb_ClientHandle != handle ) {

    LOG_D(( "D: MHIQueueBuffer failed, unknown handle.\n" ));
    return FALSE;
  }
  if ( ( ULONG ) buffer & 0x00000003 ) {

    LOG_D(( "D: MHIQueueBuffer failed, buffer not LONG aligned.\n" ));
    return FALSE;
  }

  mhiBuffer = AllocMem( sizeof( struct AmiGUS_MHI_Buffer ),
                        MEMF_ANY | MEMF_CLEAR );
  mhiBuffer->agmb_Buffer = buffer;
  mhiBuffer->agmb_BufferMax = size >> 2;
  mhiBuffer->agmb_BufferExtraBytes = size & 0x00000003;
  if ( mhiBuffer->agmb_BufferExtraBytes ) {

    LOG_W(( "W: Buffer size %ld no multiple of LONG - performance impacted!\n",
            size ));
  }
  LOG_V(( "V: Enqueueing MHI buffer 0x%08lx, current 0x%08lx, data 0x%08lx, "
          "size %ld BYTEs = %ld LONGs\n",
          mhiBuffer,
          clientHandle->agch_CurrentBuffer,
          buffer,
          size,
          mhiBuffer->agmb_BufferMax ));
  Disable();
  AddTail( buffers, ( struct Node * ) mhiBuffer );
  Enable();
  if ( !(clientHandle->agch_CurrentBuffer )) {

    clientHandle->agch_CurrentBuffer = mhiBuffer;
  }

  LOG_D(( "D: MHIQueueBuffer done\n" ));
  return TRUE;
}

ASM( APTR ) SAVEDS MHIGetEmpty(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI_Base * base )
) {

  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  struct List * buffers = ( struct List * ) &clientHandle->agch_Buffers;
  struct AmiGUS_MHI_Buffer * mhiBuffer;

  LOG_D(( "D: MHIGetEmpty start\n" ));
  for ( mhiBuffer = ( struct AmiGUS_MHI_Buffer * ) buffers->lh_Head ;
        (( clientHandle->agch_CurrentBuffer )
          && (( APTR ) clientHandle->agch_CurrentBuffer
            != ( APTR ) mhiBuffer ));
        mhiBuffer = ( struct AmiGUS_MHI_Buffer * )
          mhiBuffer->agmb_Node.mln_Succ ) {

    LOG_V(( "V: Checking for empty @ 0x%08lx\n", mhiBuffer ));
    if (( mhiBuffer ) &&
        (( struct AmiGUS_MHI_Buffer * ) buffers != mhiBuffer ) &&
        ( mhiBuffer->agmb_BufferIndex >= mhiBuffer->agmb_BufferMax )) {

      APTR result = mhiBuffer->agmb_Buffer;

      LOG_V(( "V: Removing / free'ing MHI buffer 0x%08lx, "
              "size %ld LONGs, index %ld\n",
              mhiBuffer,
              mhiBuffer->agmb_BufferMax,
              mhiBuffer->agmb_BufferIndex ));
      Disable();
      Remove(( struct Node * ) mhiBuffer );
      Enable();
      FreeMem( mhiBuffer, sizeof( struct AmiGUS_MHI_Buffer ));
      if ( mhiBuffer == clientHandle->agch_CurrentBuffer ) {

        clientHandle->agch_CurrentBuffer = NULL;
      }

      return result;
    }
  }

  LOG_D(( "D: MHIGetEmpty done\n" ));
  return NULL;
}

ASM( UBYTE ) SAVEDS MHIGetStatus(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI_Base * base )
) {
  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;

  LOG_D(( "D: MHIGetStatus %ld start/done\n", clientHandle->agch_Status ));
  return ( UBYTE ) clientHandle->agch_Status;
}

ASM( VOID ) SAVEDS MHIPlay(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI_Base * base )
) {

  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  LOG_D(( "D: MHIPlay start\n" ));
  StartAmiGusCodecPlayback();
  clientHandle->agch_Status = MHIF_PLAYING;
  LOG_D(( "D: MHIPlay done\n" ));
  return;
}

ASM( VOID ) SAVEDS MHIStop(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI_Base * base )
) {

  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  LOG_D(( "D: MHIStop start\n" ));
  StopAmiGusCodecPlayback();
  FlushAllBuffers( clientHandle );
  clientHandle->agch_Status = MHIF_STOPPED;
  LOG_D(( "D: MHIStop done\n" ));
  return;
}

ASM( VOID ) SAVEDS MHIPause(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI_Base * base )
) {

  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  LOG_D(( "D: MHIPause start\n" ));

  if ( clientHandle->agch_Status == MHIF_PAUSED ) {

    clientHandle->agch_Status = MHIF_PLAYING;

  } else if( clientHandle->agch_Status == MHIF_PLAYING ) {

    clientHandle->agch_Status = MHIF_PAUSED;
  }
  LOG_D(( "D: MHIPause done\n" ));
  return;
}

ASM( ULONG ) SAVEDS MHIQuery(
  REG( d1, ULONG query ),
  REG( a6, struct AmiGUS_MHI_Base * base )
) {

  ULONG result = MHIF_UNSUPPORTED;

  LOG_D(( "D: MHIQuery start\n" ));
  switch ( query ) {
    case MHIQ_DECODER_NAME: {

      result = ( ULONG ) "AmiGUS VS1063a codec";
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
  REG( a6, struct AmiGUS_MHI_Base * base )
) {

  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;

  LOG_D(( "D: MHISetParam start\n" ));
  LOG_D(( "V: Param %ld, Value %ld\n", param, value ));

  switch ( param ) {
    // 0=muted .. 100=0dB
    case MHIP_VOLUME:
    // 0=left .. 50=center .. 100=right
    case MHIP_PANNING:
    // 0=stereo .. 100=mono
    case MHIP_CROSSMIXING:
    // For all Equilizer:
    // 0=max.cut .. 50=unity gain .. 100=max.boost
    case MHIP_BASS: {
      
      UpdateEqualizer( VS1063_CODEC_ADDRESS_EQ5_LEVEL1, value );
      break;
    }
    case MHIP_MIDBASS: {
      
      UpdateEqualizer( VS1063_CODEC_ADDRESS_EQ5_LEVEL2, value );
      break;
    }
    case MHIP_MID: {
      
      UpdateEqualizer( VS1063_CODEC_ADDRESS_EQ5_LEVEL3, value );
      break;
    }
    case MHIP_MIDHIGH: {
      
      UpdateEqualizer( VS1063_CODEC_ADDRESS_EQ5_LEVEL4, value );
      break;
    }
    case MHIP_TREBLE: {
      
      UpdateEqualizer( VS1063_CODEC_ADDRESS_EQ5_LEVEL5, value );
      break;
    }
    case MHIP_PREFACTOR:
    default:
      LOG_W(( "W: MHISetParam cannot set param %ld to Value %ld yet\n",
              param, value ));
      break;
  }

  LOG_D(( "D: MHISetParam done\n" ));
  return;
}
