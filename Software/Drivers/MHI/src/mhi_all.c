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

VOID UpdateEqualizer( UBYTE index, UBYTE percent ) {

  APTR card = AmiGUS_MHI_Base->agb_CardBase;
  struct AmiGUS_MHI_Handle * handle = &( AmiGUS_MHI_Base->agb_ClientHandle );
  UBYTE min = 255;
  UBYTE max = 0;
  WORD gain;
  UBYTE i;
  UWORD levels[ 5 ] = {

    VS1063_CODEC_ADDRESS_EQ5_LEVEL1,
    VS1063_CODEC_ADDRESS_EQ5_LEVEL2,
    VS1063_CODEC_ADDRESS_EQ5_LEVEL3,
    VS1063_CODEC_ADDRESS_EQ5_LEVEL4,
    VS1063_CODEC_ADDRESS_EQ5_LEVEL5
  };

  // Step 1: store value if changed
  LOG_V(( "V: UpdateEqualizer index %ld was %ld, new %ld\n",
          index, handle->agch_MHI_Equalizer[ index ], percent ));
  if ( handle->agch_MHI_Equalizer[ index ] == percent ) {

    LOG_D(( "D: UpdateEQ: No change, done.\n"));
    return;
  }
  handle->agch_MHI_Equalizer[ index ] = percent;

  // Step 2: calculate min + max values for all bands
  for ( i = 0; i < 10; ++i ) {

    const UBYTE current = handle->agch_MHI_Equalizer[ i ];
    if ( current < min ) {

      min = current;
    }
    if ( current > max ) {

      max = current;
    }
  }

  // Step 3: calculate gain - remember: VERY limited in numerical range!!!
  gain = handle->agch_MHI_Equalizer[ 10 ];
  LOG_V(( "V: gain %ld, min %ld, max %ld\n", gain, min, max ));
  gain -= 50;
  gain *= 32;
  gain /= 50;
  if ( gain >= 0 ) {

    gain *= ( WORD )( 100 - max );

  } else {

    gain *= ( WORD ) min;
  }
  gain /= 50;
  LOG_V(( "V: resulting gain %ld\n", gain ));

  for ( i = 0; i < 10; i += 2 ) {

    // Originally:
    // -32 .. +32 = ((( 0 .. 100 ) * 2655 ) / 4096 ) - 32
    // gain = percent * 2655 / 4096 - 32
    // But here, we are combining 2 sliders,
    // so range is 0-200 and >> 13 to take the average :)
    WORD combined = handle->agch_MHI_Equalizer[ i ]
                    + handle->agch_MHI_Equalizer[ i + 1 ];
    LONG intermediate = (( combined * 2655 ) >> 13 ) - 32;
    WORD final = ( WORD ) intermediate + gain;
    LOG_D(( "D: UpdateEQ: Calculated gain %ld = %ld for %ld%%\n",
            intermediate, final, combined ));

    UpdateVS1063Equalizer( card, levels[ i>>1 ], final );
  }
}

VOID UpdateVolumePanning( struct AmiGUS_MHI_Handle * handle ) {

  APTR card = AmiGUS_MHI_Base->agb_CardBase;
  UBYTE volume = handle->agch_MHI_Volume;
  UBYTE panning = handle->agch_MHI_Panning;
  UBYTE multiplierLeft = ( 100 - panning ) << 1;
  UBYTE multiplierRight = (( panning - 50 ) << 1 ) + 100;
  UBYTE left;
  UBYTE right;
  UWORD value;

  LOG_D(( "D: UpdateVolume: To %lu%% vol, %lu%% pan\n", volume, panning ));

  multiplierLeft = ( multiplierLeft > 100 ) ? 100 : multiplierLeft;
  multiplierRight = ( multiplierRight > 100 ) ? 100 : multiplierRight;

  left = ( volume * multiplierLeft ) / 100;
  left = AmiGUSVolumeMapping[ left ];
  right = ( volume * multiplierRight ) / 100;
  right = AmiGUSVolumeMapping[ right ];
  value = ( left << 8 ) | ( right & 0x00FF );

  LOG_V(( "V: left x %ld / 100 = 0x%02lx, "
          "right x %ld / 100 = 0x%02lx -> 0x%04lx\n",
          multiplierLeft, left, 
          multiplierRight, right, value ));
  WriteCodecSPI( card, VS1063_CODEC_SCI_VOL, value );
}

ASM( APTR ) SAVEDS MHIAllocDecoder(
  REG( a0, struct Task * task ),
  REG( d0, ULONG signal ),
  REG( a6, struct AmiGUS_MHI * base ) 
) {

  APTR result = NULL;
  ULONG error = ENoError;
  ULONG i;

  LOG_D(( "D: MHIAllocDecoder start\n" ));

  if ( base != AmiGUS_MHI_Base ) {

    DisplayError( ELibraryBaseInconsistency );
  }
  Forbid();
  if ( AmiGUS_MHI_Base->agb_ConfigDevice->cd_Driver ) {

    error = EAmiGUSInUseError;

  } else if ( AmiGUS_MHI_Base->agb_UsageCounter ) {

    error = EDriverInUse;

  } else {

    // Now we own the card!
    ++AmiGUS_MHI_Base->agb_UsageCounter;
    AmiGUS_MHI_Base->agb_ConfigDevice->cd_Driver = ( APTR ) AmiGUS_MHI_Base;
  }
  Permit();

  if ( !error ) {
 
    APTR card = AmiGUS_MHI_Base->agb_CardBase;
    struct AmiGUS_MHI_Handle * handle = &AmiGUS_MHI_Base->agb_ClientHandle;
    handle->agch_Task = task;
    handle->agch_Signal = signal;
    handle->agch_Status = MHIF_STOPPED;
    handle->agch_CurrentBuffer = NULL;
    for ( i = 0; i < sizeof( handle->agch_MHI_Equalizer ) ; ++i ) {

      handle->agch_MHI_Equalizer[ i ] = 50;
    }
    handle->agch_MHI_Panning = 50;
    handle->agch_MHI_Volume = 50;
    NonConflictingNewMinList( &handle->agch_Buffers );

    result = ( APTR ) handle;

    LOG_D(( "D: AmiGUS MHI accepted task 0x%08lx and signal 0x%08lx.\n",
            task, signal ));

    LOG_D(( "D: Initializing VS1063 codec\n" ));
    InitVS1063Codec( card );
    InitVS1063Equalizer( card, TRUE, AmiGUSDefaultEqualizer );
    UpdateVolumePanning( handle );
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
  if (( AmiGUS_MHI_Base->agb_UsageCounter ) &&
      ( AmiGUS_MHI_Base->agb_ConfigDevice->cd_Driver
        == ( APTR ) AmiGUS_MHI_Base )) {

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
  StartAmiGusCodecPlayback();
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
  StopAmiGusCodecPlayback();
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

  struct AmiGUS_MHI_Handle * clientHandle =
    ( struct AmiGUS_MHI_Handle * ) handle;
  LOG_D(( "D: MHISetParam start\n" ));
  LOG_D(( "V: Param %ld, Value %ld\n", param, value ));

  switch ( param ) {
    // 0=muted .. 100=0dB
    case MHIP_VOLUME: {

      clientHandle->agch_MHI_Volume = value;
      UpdateVolumePanning( handle );
      break;
    }
    // 0=left .. 50=center .. 100=right
    case MHIP_PANNING: {

      clientHandle->agch_MHI_Panning = value;
      UpdateVolumePanning( handle );
      break;
    }
    // 0=stereo .. 100=mono
    case MHIP_CROSSMIXING:
    // For all Equilizer:
    // 0=max.cut .. 50=unity gain .. 100=max.boost
    case MHIP_BAND1: {

      UpdateEqualizer( 0, value );
      break;
    }
    case MHIP_BAND2: {

      UpdateEqualizer( 1, value );
      break;
    }
    case MHIP_BAND3: {

      UpdateEqualizer( 2, value );
      break;
    }
    case MHIP_BAND4: {

      UpdateEqualizer( 3, value );
      break;
    }
    case MHIP_BAND5: {

      UpdateEqualizer( 4, value );
      break;
    }
    case MHIP_BAND6: {

      UpdateEqualizer( 5, value );
      break;
    }
    case MHIP_BAND7: {

      UpdateEqualizer( 6, value );
      break;
    }
    case MHIP_BAND8: {

      UpdateEqualizer( 7, value );
      break;
    }
    case MHIP_BAND9: {

      UpdateEqualizer( 8, value );
      break;
    }
    case MHIP_BAND10: {

      UpdateEqualizer( 9, value );
      break;
    }
    case MHIP_PREFACTOR: {

      UpdateEqualizer( 10, value );
      break;
    }
    default:
      LOG_W(( "W: MHISetParam cannot set param %ld to Value %ld yet\n",
              param, value ));
      break;
  }

  LOG_D(( "D: MHISetParam done\n" ));
  return;
}
