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

#include "SDI_mhi_protos.h"
#include "amigus_codec.h"
#include "amigus_hardware.h"
#include "amigus_mhi.h"
#include "amigus_vs1063.h"
#include "debug.h"
#include "errors.h"
#include "interrupt.h"
#include "support.h"

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

VOID InitHandle( struct AmiGUS_MHI_Handle * handle ) {

  ULONG i;

  handle->agch_CardBase = handle->agch_ConfigDevice->cd_BoardAddr;

  NonConflictingNewMinList( &( handle->agch_Buffers ));
  handle->agch_CurrentBuffer = NULL;

  handle->agch_MHI_Panning = 50;
  handle->agch_MHI_Volume = 50;
  for ( i = 0; i < sizeof( handle->agch_MHI_Equalizer ) ; ++i ) {

    handle->agch_MHI_Equalizer[ i ] = 50;
  }
  handle->agch_Status = MHIF_STOPPED;
}

VOID UpdateEqualizer( APTR card,
                      struct AmiGUS_MHI_Handle * handle,
                      UBYTE index,
                      UBYTE percent ) {

  LOG_V(( "V: UpdateEqualizer index %ld was %ld, new %ld\n",
          index, handle->agch_MHI_Equalizer[ index ], percent ));

  if ( handle->agch_MHI_Equalizer[ index ] == percent ) {

    LOG_D(( "D: UpdateEQ: No change, done.\n"));
    return;
  }
  handle->agch_MHI_Equalizer[ index ] = percent;
  UpdateVS1063Equalizer( card, handle->agch_MHI_Equalizer );
}

ASM( APTR ) SAVEDS MHIAllocDecoder(
  REG( a0, struct Task * task ),
  REG( d0, ULONG signal ),
  REG( a6, struct AmiGUS_MHI * base ) 
) {

  struct AmiGUS_MHI_Handle * handle = NULL;
  ULONG error = ENoError;

  LOG_D(( "D: MHIAllocDecoder start\n" ));

  if ( base != AmiGUS_MHI_Base ) {

    error = ELibraryBaseInconsistency;
  }
  if ( !error ) {
 
    // TODO: Alloc und add to list.
    handle = &AmiGUS_MHI_Base->agb_ClientHandle;
    /*
    handle = AllocMem( sizeof( struct AmiGUS_MHI_Handle ),
                       MEMF_PUBLIC | MEMF_CLEAR );
                       */
    Forbid();
    error = FindAmiGusCodec( &( handle->agch_ConfigDevice ));
    if ( !error ) {

      handle->agch_ConfigDevice->cd_Driver = handle;
    }
    Permit();
  }
  if ( !error ) {

    InitHandle( handle );
    handle->agch_Task = task;
    handle->agch_Signal = signal;
    LOG_D(( "D: AmiGUS MHI accepted task 0x%08lx and signal 0x%08lx.\n",
            task, signal ));

    LOG_D(( "D: Initializing VS1063 codec\n" ));
    InitVS1063Codec( handle->agch_CardBase );
    InitVS1063Equalizer( handle->agch_CardBase,
                         TRUE,
                         AmiGUSDefaultEqualizer );
    UpdateVS1063VolumePanning( handle->agch_CardBase,
                               handle->agch_MHI_Volume,
                               handle->agch_MHI_Panning );
    error = CreateInterruptHandler();

  }
  if ( error ) {

    if ( handle ) {

      // FreeMem( handle, sizeof( struct AmiGUS_MHI_Handle ));
      handle = NULL;
    }

    // Takes care of the log entry, too. :)
    DisplayError( error );
  } else {

    // TODO: AddTail();
  }
  
  LOG_D(( "D: MHIAllocDecoder done\n" ));
  return handle;
}

ASM( VOID ) SAVEDS MHIFreeDecoder(
  REG( a3, APTR handle ), 
  REG( a6, struct AmiGUS_MHI * base )
) {

  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  struct Task * task = clientHandle->agch_Task;
  LONG signal = clientHandle->agch_Signal;
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
  if (( clientHandle->agch_ConfigDevice->cd_Driver == handle )) {

    clientHandle->agch_Task = NULL;
    clientHandle->agch_Signal = 0;
    clientHandle->agch_ConfigDevice->cd_Driver = NULL;

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
  REG( a6, struct AmiGUS_MHI * base )
) {
  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  struct List * buffers = ( struct List * )&clientHandle->agch_Buffers;
  struct AmiGUS_MHI_Buffer * mhiBuffer;

  LOG_D(( "D: MHIQueueBuffer start\n" ));
  if ( &AmiGUS_MHI_Base->agb_ClientHandle != clientHandle ) {

    LOG_D(( "D: MHIQueueBuffer failed, unknown handle.\n" ));
    return FALSE;
  }
  if ( ( ULONG ) buffer & 0x00000003 ) {

    LOG_D(( "D: MHIQueueBuffer failed, buffer not LONG aligned.\n" ));
    return FALSE;
  }

  mhiBuffer = AllocMem( sizeof( struct AmiGUS_MHI_Buffer ),
                        MEMF_PUBLIC | MEMF_CLEAR );
  mhiBuffer->agmb_Buffer = ( ULONG * ) buffer;
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
  REG( a6, struct AmiGUS_MHI * base )
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

      APTR result = ( APTR ) mhiBuffer->agmb_Buffer;

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
  REG( a6, struct AmiGUS_MHI * base )
) {
  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;

  LOG_D(( "D: MHIGetStatus %ld start/done\n", clientHandle->agch_Status ));
  return ( UBYTE ) clientHandle->agch_Status;
}

ASM( VOID ) SAVEDS MHIPlay(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI * base )
) {

  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  LOG_D(( "D: MHIPlay start\n" ));
  StartAmiGusCodecPlayback( clientHandle );
  clientHandle->agch_Status = MHIF_PLAYING;
  LOG_D(( "D: MHIPlay done\n" ));
  return;
}

ASM( VOID ) SAVEDS MHIStop(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI * base )
) {

  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  LOG_D(( "D: MHIStop start\n" ));
  StopAmiGusCodecPlayback( clientHandle );
  FlushAllBuffers( clientHandle );
  clientHandle->agch_Status = MHIF_STOPPED;
  LOG_D(( "D: MHIStop done\n" ));
  return;
}

ASM( VOID ) SAVEDS MHIPause(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUS_MHI * base )
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
    case MHIQ_VOLUME_CONTROL:
    case MHIQ_PANNING_CONTROL:
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
    case MHIQ_PREFACTOR_CONTROL:
    case MHIQ_BASS_CONTROL:
    case MHIQ_TREBLE_CONTROL:
    case MHIQ_MID_CONTROL:
    case MHIQ_5_BAND_EQ:
    case MHIQ_10_BAND_EQ: {

      result = MHIF_SUPPORTED;
      break;
    }
    case MHIQ_IS_68K:
    case MHIQ_IS_PPC:
    case MHIQ_CROSSMIXING:
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

  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  APTR card = clientHandle->agch_CardBase;
  UBYTE * volume = &( clientHandle->agch_MHI_Volume );
  UBYTE * panning = &( clientHandle->agch_MHI_Panning );

  LOG_D(( "D: MHISetParam start\n" ));
  LOG_D(( "V: Param %ld, Value %ld\n", param, value ));

  switch ( param ) {
    // 0=muted .. 100=0dB
    case MHIP_VOLUME: {

      *volume = value;
      UpdateVS1063VolumePanning( card, *volume, *panning );
      break;
    }
    // 0=left .. 50=center .. 100=right
    case MHIP_PANNING: {

      *panning = value;
      UpdateVS1063VolumePanning( card, *volume, *panning );
      break;
    }
    // For all Equilizer:
    // 0=max.cut .. 50=unity gain .. 100=max.boost
    case MHIP_BAND1: {

      UpdateEqualizer( card, clientHandle, 0, value );
      break;
    }
    case MHIP_BAND2: {

      UpdateEqualizer( card, clientHandle, 1, value );
      break;
    }
    case MHIP_BAND3: {

      UpdateEqualizer( card, clientHandle, 2, value );
      break;
    }
    case MHIP_BAND4: {

      UpdateEqualizer( card, clientHandle, 3, value );
      break;
    }
    case MHIP_BAND5: {

      UpdateEqualizer( card, clientHandle, 4, value );
      break;
    }
    case MHIP_BAND6: {

      UpdateEqualizer( card, clientHandle, 5, value );
      break;
    }
    case MHIP_BAND7: {

      UpdateEqualizer( card, clientHandle, 6, value );
      break;
    }
    case MHIP_BAND8: {

      UpdateEqualizer( card, clientHandle, 7, value );
      break;
    }
    case MHIP_BAND9: {

      UpdateEqualizer( card, clientHandle, 8, value );
      break;
    }
    case MHIP_BAND10: {

      UpdateEqualizer( card, clientHandle, 9, value );
      break;
    }
    case MHIP_PREFACTOR: {

      UpdateEqualizer( card, clientHandle, 10, value );
      break;
    }
    // 0=stereo .. 100=mono
    case MHIP_CROSSMIXING:
    default:
      LOG_W(( "W: MHISetParam cannot set param %ld to Value %ld yet\n",
              param, value ));
      break;
  }

  LOG_D(( "D: MHISetParam done\n" ));
  return;
}
