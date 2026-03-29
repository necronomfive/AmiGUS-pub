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

#include <stddef.h>

#include <amigus/amigus.h>

#include "amigus_pcmcia.h"
#include "amigus_private.h"
#include "amigus_zorro2.h"
#include "debug.h"
#include "SDI_amigus_protos.h"
#include "SDI_compiler.h"
#include "support.h"

/******************************************************************************
 * AmiGUS base library - private function declarations & definitions.
 *****************************************************************************/

static struct AmiGUS_Private * convertPublic2Private( struct AmiGUS * card ) {

  ULONG result = ((ULONG ) card );

  result -= offsetof( struct AmiGUS_Private, agp_AmiGUS_Public );
  LOG_D(( "D: Converted public 0x%08lx to 0x%08lx\n", card, result ));

  return (( struct AmiGUS_Private * ) result );
}

static LONG ChangePartReservation(
  APTR * ownerPointer,
  LONG which,
  APTR checkOwner,
  APTR setOwner ) {

  LOG_V(( "D: Old owner is 0x%08lx, check 0x%08lx, set 0x%08lx\n",
          *( ownerPointer ), checkOwner, setOwner ));
  if ( !( which )) {

    return AmiGUS_NoError;
  }
  if (( *( ownerPointer )) && ( *( ownerPointer) != checkOwner )) {

    return AMIGUS_IN_USE_START + which;
  }

  *( ownerPointer ) = setOwner;
  LOG_V(( "D: Now owner is 0x%08lx\n", *( ownerPointer )));

  return AmiGUS_NoError;
}

static LONG ChangeCardReservation(
  struct AmiGUS_Private * card,
  LONG which,
  APTR checkOwner,
  APTR setOwner ) {

  LONG result = AmiGUS_NoError;

  result |= ChangePartReservation( card->agp_PCM.agp_OwnerPointer,
                                   ( which & AMIGUS_FLAG_PCM ),
                                   checkOwner,
                                   setOwner );
  result |= ChangePartReservation( card->agp_Wavetable.agp_OwnerPointer,
                                   ( which & AMIGUS_FLAG_WAVETABLE ),
                                   checkOwner,
                                   setOwner );
  result |= ChangePartReservation( card->agp_Codec.agp_OwnerPointer,
                                   ( which & AMIGUS_FLAG_CODEC ),
                                   checkOwner,
                                   setOwner );

  return result;
}

static LONG ChangePartInterrupt(
  struct AmiGUS_Part * part,
  const LONG which,
  const LONG flag,
  const APTR owner,
  AmiGUS_Interrupt handler,
  APTR data ) {

  if ( !( flag & which )) {

    return AmiGUS_NoError;
  }
  if ( owner != *( part-> agp_OwnerPointer )) {

    LOG_W(( "W: Part 0x%08lx for 0x%04lx not yours\n",
            part, which ));
    return AmiGUS_NotYours;
  }

  part->agp_IntHandler = handler;
  part->agp_IntData = data;
  LOG_I(( "I: Set int for part 0x%08lx for 0x%04lx successfully\n",
          part, which ));

  return AmiGUS_NoError;
}

LONG HandleInterruptChanges( VOID ) {

  const struct List * cards = &( AmiGUS_Base->agb_Cards );
  struct AmiGUS_Private * card;
  BOOL needsZorro2Interrupt = FALSE;
  BOOL needsPcmciaInterrupt = FALSE;
  LONG result = AmiGUS_NoError;

  FOR_LIST( cards, card, struct AmiGUS_Private * ) {

    BOOL needed =
      ( NULL != card->agp_PCM.agp_IntHandler )       |
      ( NULL != card->agp_Wavetable.agp_IntHandler ) |
      ( NULL != card->agp_Codec.agp_IntHandler );

    switch ( card->agp_AmiGUS_Public.agus_TypeId ) {

      case AmiGUS_Zorro2: {

        needsZorro2Interrupt |= needed;
        break;
      }
      case AmiGUS_mini: {

        needsPcmciaInterrupt |= needed;
        break;
      }
      default: {
        LOG_E(( "E: Unknown card type in HandleInterruptChanges\n" ));
        break;
      }
    }
  }
  if (( needsZorro2Interrupt )
    && ( !( AMIGUS_BASE_F_ZORRO2_INT_SET & AmiGUS_Base->agb_Flags ))) {

    // Register Zorro2 interrupt now.
    LOG_I(( "I: Registering Zorro2 INT\n"));
    result |= AmiGusZorro2_InstallInterrupt();
  }
  if (( !( needsZorro2Interrupt ))
    && (( AMIGUS_BASE_F_ZORRO2_INT_SET & AmiGUS_Base->agb_Flags ))) {

    // Un-Register Zorro2 interrupt now.
    LOG_I(( "I: Un-registering Zorro2 INT\n"));
    result |= AmiGusZorro2_RemoveInterrupt();
  }
  if (( needsPcmciaInterrupt ) 
    && ( !( AMIGUS_BASE_F_PCMCIA_INT_SET & AmiGUS_Base->agb_Flags ))) {

    // Register PCMCIA interrupt now.
    LOG_I(( "I: Registering PCMCIA INT\n"));
    result |= AmiGusPcmcia_InstallInterrupt();
  }
  if (( !( needsPcmciaInterrupt ))
    && (( AMIGUS_BASE_F_PCMCIA_INT_SET & AmiGUS_Base->agb_Flags ))) {

    // Un-Register PCMCIA interrupt now.
    LOG_I(( "I: Un-registering PCMCIA INT\n"));
    result |= AmiGusPcmcia_RemoveInterrupt();
  }

  LOG_D(( "D: Interrupts adapted to Z2 %ld and PCMCIA %ld, result 0x%08lx\n",
          needsZorro2Interrupt, needsPcmciaInterrupt, result ));
  return result;
}

/******************************************************************************
 * AmiGUS base library - public functions.
 *****************************************************************************/

ASM( AmiGUS_PTR ) SAVEDS AmiGUS_FindCard(
  REG( a0, struct AmiGUS * card ),
  REG( a6, struct AmiGUS_Base * base )) {

  struct Node * node;
  struct AmiGUS * card_public = NULL;

  if ( !( card )) {

    node = AmiGUS_Base->agb_Cards.lh_Head;

  } else {
    
    struct AmiGUS_Private * card_private = convertPublic2Private( card );
    node = card_private->agp_Node.ln_Succ;

  }
  // List empty and end of list are the same!
  if ( node->ln_Succ != AmiGUS_Base->agb_Cards.lh_Tail ) {

    struct AmiGUS_Private * card_private = ( struct AmiGUS_Private * ) node;
    card_public = &( card_private->agp_AmiGUS_Public );
    LOG_D(( "D: Found Amigus @ 0x%08lx / 0x%08lx\n",
            card_private, card_public ));
  }

  return card_public;
}

ASM( ULONG ) SAVEDS AmiGUS_ReserveCard(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base )) {

  struct AmiGUS_Private * card_private;
  ULONG result;

  if ( NULL == card ) {

    return AmiGUS_NotFound;
  }

  card_private = convertPublic2Private( card );
  result = ChangeCardReservation( card_private, which, owner, owner );

  LOG_I(( "I: Card 0x%08lx parts 0x%04lx reserved for 0x%08lx => 0x%04lx\n",
          card, which, owner, result ));

  return result;
}

ASM( VOID ) SAVEDS AmiGUS_FreeCard(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base )) {

  struct AmiGUS_Private * card_private;
  LONG result;

  if ( NULL == card ) {

    return;
  }

  card_private = convertPublic2Private( card );
  result = ChangeCardReservation( card_private, which, owner, NULL );

  LOG_I(( "I: Card 0x%08lx parts 0x%04lx freed for 0x%08lx => 0x%04lx\n",
          card, which, owner, result ));

  return;
}

ASM( ULONG ) SAVEDS AmiGUS_InstallInterrupt(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( d2, AmiGUS_Interrupt handler ),
  REG( d3, APTR data ),
  REG( a6, struct AmiGUS_Base * base )) {

  struct AmiGUS_Private * card_private;
  ULONG result = AmiGUS_NoError;

  if ( NULL == card ) {

    return AmiGUS_NotFound;
  }

  card_private = convertPublic2Private( card );

  LOG_I(( "I: Setting interrupts for card 0x%08lx\n", card ));
  result |= ChangePartInterrupt( &( card_private->agp_PCM ),
                                 which,
                                 AMIGUS_FLAG_PCM,
                                 owner,
                                 handler,
                                 data );
  result |= ChangePartInterrupt( &( card_private->agp_Wavetable ),
                                 which,
                                 AMIGUS_FLAG_WAVETABLE,
                                 owner,
                                 handler,
                                 data );
  result |= ChangePartInterrupt( &( card_private->agp_Codec ),
                                 which,
                                 AMIGUS_FLAG_CODEC,
                                 owner,
                                 handler,
                                 data );
  LOG_I(( "I: Card 0x%08lx codec interrupts set, result 0x%04lx\n",
          card, result ));

  result |= HandleInterruptChanges();

  return result;
}

ASM( VOID ) SAVEDS AmiGUS_RemoveInterrupt(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base )) {

  struct AmiGUS_Private * card_private;
  LONG result = AmiGUS_NoError;

  if ( NULL == card ) {

    return;
  }

  card_private = convertPublic2Private( card );

  LOG_I(( "I: Freeing interrupts for card 0x%08lx\n", card ));
  result |= ChangePartInterrupt( &( card_private->agp_PCM ),
                                 which,
                                 AMIGUS_FLAG_PCM,
                                 owner,
                                 NULL,
                                 NULL );
  result |= ChangePartInterrupt( &( card_private->agp_Wavetable ),
                                 which,
                                 AMIGUS_FLAG_WAVETABLE,
                                 owner,
                                 NULL,
                                 NULL );
  result |= ChangePartInterrupt( &( card_private->agp_Codec ),
                                 which,
                                 AMIGUS_FLAG_CODEC,
                                 owner,
                                 NULL,
                                 NULL );
  LOG_I(( "I: Card 0x%08lx codec interrupts free'd, result 0x%04lx\n",
          card, result ));

  HandleInterruptChanges();

  return;
}

ASM( LONG ) /* __entry for vbcc ? */ SAVEDS INTERRUPT HandleInterrupt (
  REG( a1, struct AmiGUS_Base * base )) {

  const struct List * cards = &( AmiGUS_Base->agb_Cards );
  struct AmiGUS_Private * card;
  LONG result = 0;

  FOR_LIST( cards, card, struct AmiGUS_Private * ) {

    const AmiGUS_Interrupt pcmHandler = card->agp_PCM.agp_IntHandler;
    const APTR pcmData = card->agp_PCM.agp_IntData;
    const AmiGUS_Interrupt wtHandler = card->agp_Wavetable.agp_IntHandler;
    const APTR wtData = card->agp_Wavetable.agp_IntData;
    const AmiGUS_Interrupt codecHandler = card->agp_Codec.agp_IntHandler;
    const APTR codecData = card->agp_Codec.agp_IntData;

    if ( NULL != pcmHandler ) {

      LOG_INT(( "INT: Sending INT to PCM handler 0x%08lx\n", pcmHandler ));
      result |= pcmHandler( pcmData );
      LOG_INT(( "INT: PCM handler result %lx\n", result ));
    }

    if ( NULL != wtHandler ) {

      LOG_INT(( "INT: Sending INT to Wavetable handler 0x%08lx\n", wtHandler ));
      result |= wtHandler( wtData );
      LOG_INT(( "INT: Wavetable handler result %lx\n", result ));
    }

    if ( NULL != codecHandler ) {

      LOG_INT(( "INT: Sending INT to Codec handler 0x%08lx\n", codecHandler ));
      result |= codecHandler( codecData );
      LOG_INT(( "INT: Codec handler result %lx\n", result ));
    }
    LOG_INT(( "INT: Card %s at 0x%08lx done\n",
              card->agp_AmiGUS_Public.agus_TypeName,
              card ));
  }

  return result;
}
