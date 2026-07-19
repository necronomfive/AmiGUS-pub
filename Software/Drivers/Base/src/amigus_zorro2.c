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

#include <amigus/amigus.h>

#include <hardware/intbits.h>

#include <proto/expansion.h>
#include <proto/exec.h>

#include "amigus_hardware.h"
#include "amigus_private.h"
#include "amigus_zorro2.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "SDI_amigus_protos.h"

#define ANY_PRODUCT_ID   -1

STRPTR AmiGUS_Zorro2_Name = "AmiGUS Zorro2";

/******************************************************************************
 * AmiGUS Zorro2 functions - public function definitions.
 *****************************************************************************/

VOID AmiGusZorro2_AddAll( struct List * cards ) {

  struct AmiGUS_Base * base = AmiGUS_Base;
  struct ConfigDev * cd_PCM = NULL;
  struct ConfigDev * cd_Wavetable = NULL;
  struct ConfigDev * cd_Codec = NULL;
  LONG count = 0;
  
  for ( ; ; ) {

    struct AmiGUS_Private * cardPrivate;
    struct AmiGUS * cardPublic;
    ULONG serialPCM;
    ULONG serialWavetable;
    ULONG serialCodec;
    ULONG serial;

    cd_PCM = FindConfigDev( cd_PCM,
                            AMIGUS_MANUFACTURER_ID,
                            AMIGUS_MAIN_PRODUCT_ID );
    if ( !( cd_PCM )) {

      break;
    }
    cd_Wavetable = FindConfigDev( cd_Wavetable,
                                  AMIGUS_MANUFACTURER_ID,
                                  AMIGUS_HAGEN_PRODUCT_ID );
    if ( !( cd_Wavetable )) {

      break;
    }
    cd_Codec = FindConfigDev( cd_Codec,
                              AMIGUS_MANUFACTURER_ID,
                              AMIGUS_CODEC_PRODUCT_ID );
    if ( !( cd_Codec )) {

      break;
    }

    cardPrivate = AllocMem( sizeof( struct AmiGUS_Private ), MEMF_ANY );

    cardPrivate->agp_PCM.agp_OwnerPointer = &( cd_PCM->cd_Driver );
    cardPrivate->agp_PCM.agp_IntHandler = NULL;
    cardPrivate->agp_PCM.agp_IntData = NULL;

    cardPrivate->agp_Wavetable.agp_OwnerPointer = &( cd_Wavetable->cd_Driver );
    cardPrivate->agp_Wavetable.agp_IntHandler = NULL;
    cardPrivate->agp_Wavetable.agp_IntData = NULL;

    cardPrivate->agp_Codec.agp_OwnerPointer = &( cd_Codec->cd_Driver );
    cardPrivate->agp_Codec.agp_IntHandler = NULL;
    cardPrivate->agp_Codec.agp_IntData = NULL;

    cardPublic = &( cardPrivate->agp_AmiGUS_Public );
    cardPublic->agus_PcmBase = cd_PCM->cd_BoardAddr;
    cardPublic->agus_WavetableBase = cd_Wavetable->cd_BoardAddr;
    cardPublic->agus_CodecBase = cd_Codec->cd_BoardAddr;

    cardPublic->agus_FpgaId.idLongs[ 0 ] = ReadReg32( cardPublic->agus_PcmBase,
                                                      AMIGUS_FPGA_ID_LOW );
    cardPublic->agus_FpgaId.idLongs[ 1 ] = ReadReg32( cardPublic->agus_PcmBase,
                                                      AMIGUS_FPGA_ID_HIGH );

    cardPublic->agus_TypeId = AmiGUS_Zorro2;
    cardPublic->agus_TypeName = AmiGUS_Zorro2_Name;

    cardPublic->agus_HardwareRev = 0;

    serialPCM = cd_PCM->cd_Rom.er_SerialNumber;
    serialWavetable = cd_Wavetable->cd_Rom.er_SerialNumber;
    serialCodec = cd_Codec->cd_Rom.er_SerialNumber;

    if (( serialPCM != serialWavetable )
      || ( serialPCM != serialCodec )) {

      LOG_W(( "W: Versions 0x%08lx/0x%08lx/0x%08lx of AmiGUSs "
              "combined into 0x%08lx/0x%08lx do not match!\n",
              serialPCM, serialWavetable, serialCodec,
              cardPrivate, cardPublic ));
    }

    serial = MIN( serialPCM, MIN( serialWavetable, serialCodec ));
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

    ++count;
    AddTail( cards, &( cardPrivate->agp_Node ));
  }
  if ( count ) {

    LOG_D(( "D: Found %ld AmiGUS Zorro2 cards.\n", count ));
    base->agb_Interrupt = AllocMem( sizeof( struct Interrupt ),
                                    MEMF_ANY | MEMF_CLEAR );
    base->agb_Interrupt->is_Node.ln_Pri = 100;
    base->agb_Interrupt->is_Node.ln_Name = "AmiGUS_Base_INT";
    base->agb_Interrupt->is_Data = ( APTR ) base;
    base->agb_Interrupt->is_Code = ( VOID ( * )( )) HandleInterrupt;
    LOG_D(( "D: Zorro2 interrupt prepared.\n" ));

  } else {

    LOG_D(( "D: No AmiGUS Zorro2 cards found.\n" ));
    base->agb_Interrupt = NULL;
  }

  return;
}

VOID AmiGusZorro2_RemoveAll( struct List * cards ) {

  struct AmiGUS_Base * base = AmiGUS_Base;
  struct AmiGUS_Private * cardPrivate; 
  FOR_LIST( cards, cardPrivate, struct AmiGUS_Private * ) {

    if ( AmiGUS_Zorro2 == cardPrivate->agp_AmiGUS_Public.agus_TypeId ) {

      LOG_I(( "I: Releasing %s ...\n", AmiGUS_Zorro2_Name ));
      *( cardPrivate->agp_PCM.agp_OwnerPointer ) = NULL;
      *( cardPrivate->agp_Wavetable.agp_OwnerPointer ) = NULL;
      *( cardPrivate->agp_Codec.agp_OwnerPointer ) = NULL;
      LOG_I(( "I: done, %s free'd!\n", AmiGUS_Zorro2_Name ));
    }
  }
  if( base->agb_Interrupt ) {

    FreeMem( base->agb_Interrupt, sizeof( struct Interrupt ));
    base->agb_Interrupt = NULL;
    LOG_D(( "D: Free'd Zorro2 card interrupt.\n" ));
  }
}

LONG AmiGusZorro2_InstallInterrupt( VOID ) {

  Disable();
  if ( AmiGUS_Base->agb_Flags & AMIGUS_BASE_F_ZORRO2_INT_SET ) {

    Enable();
    LOG_W(( "W: Zorro2 interrupt already installed.\n" ));
    return AmiGUS_InterruptInstallFailed;
  }
  AmiGUS_Base->agb_Flags |= AMIGUS_BASE_F_ZORRO2_INT_SET;
  AddIntServer( INTB_PORTS, AmiGUS_Base->agb_Interrupt );

  Enable();
  LOG_I(( "I: Zorro2 interrupt successfully installed.\n" ));
  return AmiGUS_NoError;
}

LONG AmiGusZorro2_RemoveInterrupt( VOID ) {

  Disable();
  if ( !( AmiGUS_Base->agb_Flags & AMIGUS_BASE_F_ZORRO2_INT_SET )) {

    Enable();
    LOG_W(( "W: Zorro2 interrupt not installed.\n" ));
    return AmiGUS_InterruptRemoveFailed;
  }
  AmiGUS_Base->agb_Flags &= ~AMIGUS_BASE_F_ZORRO2_INT_SET;
  RemIntServer( INTB_PORTS, AmiGUS_Base->agb_Interrupt );

  Enable();
  LOG_I(( "I: Zorro2 interrupt successfully removed.\n" ));
  return AmiGUS_NoError;
}
