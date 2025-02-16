#include <libraries/mhi.h>

#include "amigus_mhi.h"
#include "debug.h"
#include "library.h"
#include "SDI_mhi_protos.h"

ASM( APTR ) SAVEDS MHIAllocDecoder(
  REG( a0, struct Task * task ),
  REG( d0, ULONG signal ),
  REG( a6, struct AmiGUSBase * amiGUSBase ) 
) {

  APTR result = NULL;

  LOG_D(( "D: MHIAllocDecoder start\n" ));

  if ( !AmiGUSBase->agb_UsageCounter ) {

    struct AmiGUSClientHandle * handle = &AmiGUSBase->agb_ClientHandle;
    handle->agch_Task = task;
    handle->agch_Signal = signal;
    result = ( APTR ) handle;
    ++AmiGUSBase->agb_UsageCounter;

    LOG_D(( "D: AmiGUS MHI accepted task 0x%08lx and signal 0x%08lx.\n",
            task, signal ));

  } else {

    LOG_D(( "D: AmiGUS MHI in use, denying.\n" ));
  }
  
  LOG_D(( "D: MHIAllocDecoder done\n" ));
  return result;
}

ASM( VOID ) SAVEDS MHIFreeDecoder(
  REG( a3, APTR handle ), 
  REG( a6, struct AmiGUSBase * amiGUSBase )
) {

  struct AmiGUSClientHandle *clientHandle =
    ( struct AmiGUSClientHandle * )handle;
  struct Task * task = clientHandle->agch_Task;
  ULONG signal = clientHandle->agch_Signal;

  LOG_D(( "D: MHIFreeDecoder start\n" ));

  if (( &AmiGUSBase->agb_ClientHandle == clientHandle ) &&
      ( task ) &&
      ( AmiGUSBase->agb_UsageCounter )) {

    clientHandle->agch_Task = NULL;
    clientHandle->agch_Signal = 0;
    --AmiGUSBase->agb_UsageCounter;

    LOG_D(( "D: AmiGUS MHI free'd up task 0x%08lx and signal 0x%08lx.\n",
            task, signal ));

  } else {

    LOG_W(( "W: AmiGUS MHI does not know task 0x%08lx and signal 0x%08lx"
            " - hence not free'd.\n",
            task, signal ));
  }
  LOG_D(( "D: MHIFreeDecoder done\n" ));
  return;
}

ASM( BOOL ) SAVEDS MHIQueueBuffer(
  REG( a3, APTR handle ),
  REG( a0, APTR buffer ),
  REG( d0, ULONG size),
  REG( a6, struct AmiGUSBase * amiGUSBase )
) {
  LOG_D(( "D: MHIQueueBuffer start\n" ));
  LOG_D(( "D: MHIQueueBuffer done\n" ));
  return FALSE;
}

ASM( APTR ) SAVEDS MHIGetEmpty(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUSBase * amiGUSBase )
) {
  LOG_D(( "D: MHIGetEmpty start\n" ));
  LOG_D(( "D: MHIGetEmpty done\n" ));
  return NULL;
}

ASM( UBYTE ) SAVEDS MHIGetStatus(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUSBase * amiGUSBase )
) {
  LOG_D(( "D: MHIGetStatus start\n" ));
  LOG_D(( "D: MHIGetStatus done\n" ));
  return 0;
}

ASM( VOID ) SAVEDS MHIPlay(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUSBase * amiGUSBase )
) {
  LOG_D(( "D: MHIPlay start\n" ));
  LOG_D(( "D: MHIPlay done\n" ));
  return;
}

ASM( VOID ) SAVEDS MHIStop(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUSBase * amiGUSBase )
) {
  LOG_D(( "D: MHIStop start\n" ));
  LOG_D(( "D: MHIStop done\n" ));
  return;
}

ASM( VOID ) SAVEDS MHIPause(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUSBase * amiGUSBase )
) {
  LOG_D(( "D: MHIPause start\n" ));
  LOG_D(( "D: MHIPause done\n" ));
  return;
}

ASM( ULONG ) SAVEDS MHIQuery(
  REG( d1, ULONG query ),
  REG( a6, struct AmiGUSBase * amiGUSBase )
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
  LOG_D(( "D: MHIQuery done\n" ));
  return result;
}

ASM( VOID ) SAVEDS MHISetParam(
  REG( a3, APTR handle ),
  REG( d0, UWORD param ),
  REG( d1, ULONG value ),
  REG( a6, struct AmiGUSBase * amiGUSBase )
) {
  LOG_D(( "D: MHISetParam start\n" ));
  LOG_D(( "D: MHISetParam done\n" ));
  return;
}
