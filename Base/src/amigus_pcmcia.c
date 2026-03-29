/*
 * This file is part of the amigus.library.
 *
 * amigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * amigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with amigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <resources/card.h>

#include <proto/cardres.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include <amigus/amigus.h>

#include "amigus_hardware.h"
#include "amigus_pcmcia.h"
#include "amigus_private.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "SDI_amigus_protos.h"

/*
 * defines are limited to 32 chars due to a SAS/C insufficiency !!!
 *
 * So define below is just kind of a ruler...
 */
#define SASC_MAXIMUM_DEFINE_LENGTH_IS_32 12345678
#define GAYLE_CONFIG_BASE_ADDRESS        0x00DA8000
#define GAYLE_CONFIG_MAX_ADDRESS         0x00DAffFF
#define PCMCIA_MIN_MEMORY_ADDRESS        0x00600000
#define PCMCIA_MAX_MEMORY_ADDRESS        0x009FffFF
#define PCMCIA_MEMORY_SCAN_GRANULARITY   0x00010000
#define PCMCIA_ACCESS_SPEED_NS           200

#define GAYLE_REG_ASK_HEDLEY             0x1000

#define GAYLE_PCMCIA_ASK_HEDLEY_RESET    0x0300

#define CHAR_TO_ULONG( a, b, c, d )      ((( a ) << 24 ) | (( b ) << 16 ) | (( c ) << 8 ) | ( d ))
#define AMIGUS_MINI_CARD_ID_LOW          CHAR_TO_ULONG( 'A', 'M' ,'I', 'G' )
#define AMIGUS_MINI_CARD_ID_HIGH         CHAR_TO_ULONG( 'U', 'S' ,'M', 'N' )
#define AMIGUS_MINI_ID_LOW_OFFSET        0x80
#define AMIGUS_MINI_ID_HIGH_OFFSET       0x84
#define AMIGUS_MINI_SERIAL_OFFSET        0x88
#define AMIGUS_MINI_HARDWARE_ID          0x8C
#define AMIGUS_MINI_PCM_OFFSET           0x00000100
#define AMIGUS_MINI_CODEC_OFFSET         0x00000200
#define AMIGUS_MINI_WAVETABLE_OFFSET     0x00000300

extern const char LibName[];

STRPTR AmiGUS_Mini_Name = "AmiGUS mini";

ASM( LONG ) /* __entry for vbcc ? */ SAVEDS INTERRUPT HandleRemovedInterrupt (
  REG( a1, struct AmiGUS_Base * base )) {

  LOG_INT(( "INT: PCMCIA removed.\n" ));

  return 0;
}

ASM( LONG ) /* __entry for vbcc ? */ SAVEDS INTERRUPT HandleInsertedInterrupt (
  REG( a1, struct AmiGUS_Base * base )) {

  LOG_INT(( "INT: PCMCIA inserted.\n" ));

  return 0;
}

ASM( ULONG ) /* __entry for vbcc ? */ SAVEDS INTERRUPT HandleStatusInterrupt (
  REG( d0, ULONG status ),
  REG( a1, struct AmiGUS_Base * base )) {

  LOG_INT(( "INT: PCMCIA status change, now %ld.\n", status ));
  if ( CARD_STATUSF_IRQ & status ) {

    HandleInterrupt( base );
  }
  return status;
}

static VOID ResetPcmcia( VOID ) {

  UWORD value;
  
  LOG_I(( "I: Triggering PCMCIA reset.\n" ));

  value = ReadReg16(( APTR ) GAYLE_CONFIG_BASE_ADDRESS,
                             GAYLE_REG_ASK_HEDLEY );

  WriteReg16(( APTR ) GAYLE_CONFIG_BASE_ADDRESS,
              GAYLE_REG_ASK_HEDLEY,
              value | GAYLE_PCMCIA_ASK_HEDLEY_RESET );
  WriteReg16(( APTR ) GAYLE_CONFIG_BASE_ADDRESS,
              GAYLE_REG_ASK_HEDLEY,
              value );
}

static struct CardHandle * CreateCardHandle( VOID ) {

  struct CardHandle * handle;

  handle = AllocMem( sizeof( struct CardHandle ),
                     MEMF_ANY | MEMF_CLEAR );
  if ( !( handle )) {

    LOG_E(( "E: Cannot allocate CardHandle!\n" ));
    return handle;
  }

  handle->cah_CardRemoved = AllocMem( sizeof( struct Interrupt ),
                                      MEMF_ANY | MEMF_CLEAR );
  if ( !( handle->cah_CardRemoved )) {

    LOG_E(( "E: Cannot allocate Interrupt for CardRemoved!\n" ));
    return NULL;
  }

  handle->cah_CardInserted = AllocMem( sizeof( struct Interrupt ),
                                       MEMF_ANY | MEMF_CLEAR );
  if ( !( handle->cah_CardInserted )) {

    LOG_E(( "E: Cannot allocate Interrupt for CardInserted!\n" ));
    return NULL;
  }
  handle->cah_CardStatus = AllocMem( sizeof( struct Interrupt ),
                                     MEMF_ANY | MEMF_CLEAR );
  if ( !( handle->cah_CardStatus )) {

    LOG_E(( "E: Cannot allocate Interrupt for CardStatus!\n" ));
    return NULL;
  }
  handle->cah_CardNode.ln_Pri = 20;
  handle->cah_CardNode.ln_Name = ( char * ) LibName;
  handle->cah_CardRemoved->is_Data = ( APTR ) AmiGUS_Base;
  handle->cah_CardRemoved->is_Code = ( VOID ( * )( )) HandleRemovedInterrupt;
  handle->cah_CardInserted->is_Data = ( APTR ) AmiGUS_Base;
  handle->cah_CardInserted->is_Code = ( VOID ( * )( )) HandleInsertedInterrupt;
  handle->cah_CardStatus->is_Data = ( APTR ) AmiGUS_Base;
  handle->cah_CardStatus->is_Code = ( VOID ( * )( )) HandleStatusInterrupt;
  handle->cah_CardFlags = CARDF_RESETREMOVE;
  return handle;
}

static BOOL IsZorro2SpaceFree( VOID ) {

  ULONG address;
  for ( address = PCMCIA_MIN_MEMORY_ADDRESS; 
        address <= PCMCIA_MAX_MEMORY_ADDRESS;
        address += PCMCIA_MEMORY_SCAN_GRANULARITY ) {

    if ( TypeOfMem(( APTR ) address )) {

      LOG_D(( "D: PCMCIA space in use at 0x%08lx.\n", address ));
      return FALSE;
    }
  }
  LOG_D(( "D: PCMCIA space seems free.\n" ));
  return TRUE;
}

static BOOL ActivateMemoryMode( struct CardHandle * handle ) {

  ULONG speed;
  if ( !( IsZorro2SpaceFree( ))) {

    LOG_I(( "I: PCMCIA memory area blocked, continuing without!\n" ));
    return FALSE;
  }
  
  speed = CardAccessSpeed( handle, PCMCIA_ACCESS_SPEED_NS );
	if ( !( speed )) {	

    LOG_W(( "W: Setting card access speed failed, continuing without!\n" ));
    return FALSE;
  }
  Delay( 3 );

  if ( !( BeginCardAccess( handle ))) {

    LOG_W(( "W: Cannot begin card memory access, continuing without!\n" ));
    return FALSE;
  }
  AmiGUS_Base->agb_Flags |= AMIGUS_BASE_F_PCMCIA_MEMORY_MODE;
  LOG_I(( "I: Switched card to memory access with %ldns access.\n", speed ));

  return TRUE;
}

static struct AmiGUS_Private * CreateCardPrivate( ULONG cardBase ) {

  struct AmiGUS_Private * cardPrivate;
  struct AmiGUS * cardPublic;
  APTR ownerPcm;
  APTR ownerWavetable;
  APTR ownerCodec;
  APTR cardPcm;
  APTR cardWavetable;
  APTR cardCodec;
  ULONG serial;

  cardPrivate = AllocMem( sizeof( struct AmiGUS_Private ), MEMF_ANY );
  if ( !( cardPrivate )) {

    LOG_E(( "E: Could not allocate private PCMCIA card structure.\n" ));
    return cardPrivate;
  }

  ownerPcm = &( cardPrivate->agp_PCM.agp_MaybeOwnerData );
  cardPrivate->agp_PCM.agp_OwnerPointer = ownerPcm;
  cardPrivate->agp_PCM.agp_MaybeOwnerData = NULL;
  cardPrivate->agp_PCM.agp_IntHandler = NULL;
  cardPrivate->agp_PCM.agp_IntData = NULL;

  ownerWavetable = &( cardPrivate->agp_Wavetable.agp_MaybeOwnerData );
  cardPrivate->agp_Wavetable.agp_OwnerPointer = ownerWavetable;
  cardPrivate->agp_Wavetable.agp_MaybeOwnerData = NULL;
  cardPrivate->agp_Wavetable.agp_IntHandler = NULL;
  cardPrivate->agp_Wavetable.agp_IntData = NULL;

  ownerCodec = &( cardPrivate->agp_Codec.agp_MaybeOwnerData );
  cardPrivate->agp_Codec.agp_OwnerPointer = ownerCodec;    
  cardPrivate->agp_Codec.agp_MaybeOwnerData = NULL;
  cardPrivate->agp_Codec.agp_IntHandler = NULL;
  cardPrivate->agp_Codec.agp_IntData = NULL;

  cardPublic = &( cardPrivate->agp_AmiGUS_Public );

  cardPcm = ( APTR )( cardBase + AMIGUS_MINI_PCM_OFFSET );
  cardPublic->agus_PcmBase = cardPcm;
  cardWavetable = ( APTR )( cardBase + AMIGUS_MINI_WAVETABLE_OFFSET );
  cardPublic->agus_WavetableBase = cardWavetable;
  cardCodec = ( APTR )( cardBase + AMIGUS_MINI_CODEC_OFFSET );
  cardPublic->agus_CodecBase = cardCodec;

  cardPublic->agus_FpgaId.idLongs[ 0 ] = ReadReg32( cardPublic->agus_PcmBase,
                                                    AMIGUS_FPGA_ID_LOW );
  cardPublic->agus_FpgaId.idLongs[ 1 ] = ReadReg32( cardPublic->agus_PcmBase,
                                                    AMIGUS_FPGA_ID_HIGH );

  cardPublic->agus_TypeId = AmiGUS_mini;
  cardPublic->agus_TypeName = AmiGUS_Mini_Name;

  cardPublic->agus_HardwareRev = ReadReg16(( APTR ) cardBase,
                                            AMIGUS_MINI_HARDWARE_ID );

  serial = ReadReg32(( APTR ) cardBase, AMIGUS_MINI_SERIAL_OFFSET );
  cardPublic->agus_FirmwareRev = serial;
  LOG_V(("I: AmiGUS firmware 0x%08lx\n", serial ));

  cardPublic->agus_Minute = ( UBYTE )(( serial & 0x0000003Ful )       );
  cardPublic->agus_Hour   = ( UBYTE )(( serial & 0x000007C0ul ) >>  6 );
  cardPublic->agus_Day    = ( UBYTE )(( serial & 0x0000F800ul ) >> 11 );
  cardPublic->agus_Month  = ( UBYTE )(( serial & 0x000F0000ul ) >> 16 );
  cardPublic->agus_Year   = ( UWORD )(( serial & 0xFFF00000ul ) >> 20 );
  LOG_I(( "I: AmiGUS firmware date %04ld-%02ld-%02ld, %02ld:%02ld\n",
          cardPublic->agus_Year, 
          cardPublic->agus_Month,
          cardPublic->agus_Day,
          cardPublic->agus_Hour,
          cardPublic->agus_Minute ));

  return cardPrivate;
}

VOID AmiGusPcmcia_AddAll( struct List * cards ) {

  struct AmiGUS_Base * base = AmiGUS_Base;
  struct AmiGUS_Private * card_private;
  struct CardMemoryMap * cardMap;
  union { ULONG idLong[ 2 ];
          UBYTE idString[ 9 ];
        } id;
  ULONG interface;
  UBYTE status;
  APTR own;
  BOOL reset;

  base->agb_CardHandle = NULL;
  if ( !( base->agb_CardResource )) {

    LOG_I(( "I: No card.resource, not looking further on PCMCIA.\n" ));
    return;
  }
  LOG_I(( "I: Found card.resource %ld.%ld.\n",
          base->agb_CardResource->lib_Version,
          base->agb_CardResource->lib_Revision ));

  interface = CardInterface();
  if ( CARD_INTERFACE_AMIGA_0	!= interface ) {

    LOG_I(( "I: Unknown PCMCIA interface, not looking further.\n" ));
    return;
  }

  ResetPcmcia();

  status = ReadCardStatus();
  if ( !( CARD_STATUSF_CCDET & status )) {

    LOG_I(( "I: No card inserted.\n" ));
    return;
  }

  base->agb_CardHandle = CreateCardHandle();
  if ( !( base->agb_CardHandle )) {

    return;
  }

  LOG_D(( "D: Requesting PCMCIA card ownership for 0x%08lx.\n",
          base->agb_CardHandle ));
  own = OwnCard( base->agb_CardHandle );
  if ( NULL != own ) {

    LOG_E(( "E: Failed owning card!\n" ));
    return;
  }
  LOG_D(( "D: Success, owning card!\n" ));

  reset = CardResetCard( base->agb_CardHandle );
  if ( !( reset )) {

    LOG_E(( "E: Card reset failed!\n" ));
    ReleaseCard( base->agb_CardHandle, 0 );
    return;
  }
  Delay( 3 );

  cardMap = GetCardMap(); // <- has addresses and sizes and shit
  LOG_D(( "D: Addresses: Mem 0x%08lx Attr 0x%08lx IO 0x%08lx\n",
          cardMap->cmm_CommonMemory,
          cardMap->cmm_AttributeMemory,
          cardMap->cmm_IOMemory ));

  if ( 39 <= base->agb_CardResource->lib_Version ) {

    LOG_V(( "V: Sizes:     Mem 0x%08lx Attr 0x%08lx IO 0x%08lx\n",
            cardMap->cmm_CommonMemSize,
            cardMap->cmm_AttributeMemSize,
            cardMap->cmm_IOMemSize ));
  }

  id.idLong[ 0 ] = ReadReg32( cardMap->cmm_AttributeMemory,
                              AMIGUS_MINI_ID_LOW_OFFSET );
  id.idLong[ 1 ] = ReadReg32( cardMap->cmm_AttributeMemory, 
                              AMIGUS_MINI_ID_HIGH_OFFSET );
  id.idString[ 8 ] = 0;
  LOG_I(( "I: Found card %s.\n", id.idString ));
  if (( AMIGUS_MINI_CARD_ID_LOW  != id.idLong[ 0 ] ) ||
      ( AMIGUS_MINI_CARD_ID_HIGH != id.idLong[ 1 ] )) {

    LOG_E(( "E: No %s found!\n", AmiGUS_Mini_Name ));
    ReleaseCard( base->agb_CardHandle, CARDB_REMOVEHANDLE );
    return;
  }

  //ActivateMemoryMode( handle );
  if ( AMIGUS_BASE_F_PCMCIA_MEMORY_MODE & base->agb_Flags ) {

    card_private = CreateCardPrivate(( ULONG ) cardMap->cmm_CommonMemory );

  } else {

    card_private = CreateCardPrivate(( ULONG ) cardMap->cmm_AttributeMemory );
  }
  if ( card_private ) {

    AddTail( cards, &( card_private->agp_Node ));
    LOG_I(( "I: AmiGUS mini found and added.\n"));
  }

  return;
}

VOID AmiGusPcmcia_RemoveAll( struct List * cards ) {

  struct AmiGUS_Base * base = AmiGUS_Base;
  struct AmiGUS_Private * card_private;

  if ( AMIGUS_BASE_F_PCMCIA_MEMORY_MODE & base->agb_Flags ) {

    EndCardAccess( base->agb_CardHandle );
  }
  FOR_LIST( cards, card_private, struct AmiGUS_Private * ) {

    if ( AmiGUS_mini == card_private->agp_AmiGUS_Public.agus_TypeId ) {

      LOG_I(( "I: Resetting %s...\n", AmiGUS_Mini_Name ));
      CardResetCard( base->agb_CardHandle );
      Delay( 3 );
      LOG_I(( "I: done, releasing %s...\n", AmiGUS_Mini_Name ));
      Delay( 3 );
      ReleaseCard( base->agb_CardHandle, CARDB_REMOVEHANDLE );
      LOG_I(( "I: done, %s free'd!\n", AmiGUS_Mini_Name ));
    }
  }
  if ( base->agb_CardHandle ) {

    if ( base->agb_CardHandle->cah_CardRemoved ) {

      FreeMem( base->agb_CardHandle->cah_CardRemoved,
               sizeof( struct Interrupt ));
    }
    if ( base->agb_CardHandle->cah_CardInserted ) {

      FreeMem( base->agb_CardHandle->cah_CardInserted,
               sizeof( struct Interrupt ));
    }
    if ( base->agb_CardHandle->cah_CardStatus ) {

      FreeMem( base->agb_CardHandle->cah_CardStatus,
               sizeof( struct Interrupt ));
    }
    FreeMem( base->agb_CardHandle,
             sizeof( struct CardHandle ));
    base->agb_CardHandle = NULL;
    LOG_D(( "D: Free'd PCMCIA card handle and interrupts.\n" ));
  }
}

LONG AmiGusPcmcia_InstallInterrupt( VOID ) {

  LOG_I(( "I: PCMCIA ints handled permanently, no install needed.\n" ));

  return AmiGUS_NoError;
}

LONG AmiGusPcmcia_RemoveInterrupt( VOID ) {

  LOG_I(( "I: PCMCIA ints handled permanently, no remove possible.\n" ));

  return AmiGUS_NoError;
}
