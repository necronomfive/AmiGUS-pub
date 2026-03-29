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

#ifndef AMIGUS_PRIVATE_H
#define AMIGUS_PRIVATE_H

/*
 * AmiGUS library header.
 *
 * To be used only internally - but there in all .c files!
 * If you are using some of the library base addresses from
 * BASE_REDEFINE directly, e.g. in library.c,
 * do a #define NO_BASE_REDEFINE before including this file.
 */

/* Activate / De-activate this define to toggle lib base mode! */
#define BASE_GLOBAL /**/

#ifndef BASE_GLOBAL 
#ifndef NO_BASE_REDEFINE
/* 
 * either this is active for everything except library.c
 * or BASE_GLOBAL is active everywhere
 */
#define BASE_REDEFINE
#endif
#endif

#include <resources/card.h>

#include <amigus/amigus.h>

#include "library.h"

/*
 * Not yet query-able facts, maybe shall have a function for that later? <- TODO
 */
#define AMIGUS_AUTHOR               "Christoph `Chritoph` Fassbach"
#define AMIGUS_COPYRIGHT            "(c) 2025 Christoph Fassbach / LGPL3"
#define AMIGUS_ANNOTATION           "Thanks to: Oliver Achten (AmiGUS), " \
                                    "Frank Wille (vbcc)"
#define AMIGUS_VERSION              LIBRARY_IDSTRING

/*
 * If logging to memory is activated, this is used to mark the start
 * of the log memory. And as 1 pointer to this marker, 1 pointer to the
 * library file name and 1 more pointer to the marker are used, the full
 * start marker is not even in the library, no need to unload the library
 * so even with library in memory it should unique in memory.
 */
#define AMIGUS_MEM_LOG_BORDERS      "********************************"

/*
 * defines are limited to 32 chars due to a SAS/C insufficiency !!!
 *
 * So define below is just kind of a ruler...
 */
//      SASC_MAXIMUM_DEFINE_LENGTH_IS_32 12345678

/**
 * List of potential flags managing library state in AmiGUS_Base->agb_Flags.
 */
#define AMIGUS_BASE_F_ZORRO2_INT_SET     0x00000001 // Z2 interrupt installed,
#define AMIGUS_BASE_F_PCMCIA_INT_SET     0x00000002 // same for PCMCIA
#define AMIGUS_BASE_F_PCMCIA_MEMORY_MODE 0x00000004 // PCMCIA access as memory

/******************************************************************************
 * Library base structure
 *****************************************************************************/

/**
 * Private AmiGUS library base structure.
 *
 * There is no public one, pointers to libraries opened, interrupts, logs.
 * Nothing to play around with.
 */
struct AmiGUS_Base {
  /* Library base stuff */
  struct BaseLibrary          agb_BaseLibrary;   // Instance of library.h

  struct ExecBase           * agb_SysBase;       // Exec, allocations etc.
  struct DosLibrary         * agb_DOSBase;       // DOS, logs and so on
  struct Library            * agb_ExpansionBase; // Finding devices
  struct Library            * agb_CardResource;  // PCMCIA support

  /* AmiGUS specific member variables */
  struct List                 agb_Cards;         // List of AmiGUS_Privates
  struct Interrupt          * agb_Interrupt;     // Struct for Zorro2 interrupts
  struct CardHandle         * agb_CardHandle;    // Struct for card.resource
  ULONG                       agb_Flags;         // See list of flags above!

  BPTR                        agb_LogFile;       // Debug log file handle
  APTR                        agb_LogMem;        // Debug log memory blob
};

/**
 * Private AmiGUS card's functional block representation.
 * Each AmiGUS card has three of these, PCM, Wavetable, Codec,
 * and each of the parts has the same properties.
 */
struct AmiGUS_Part {
  APTR                      * agp_OwnerPointer;   // To real owner data
  APTR                        agp_MaybeOwnerData; // Not for Zorro2 ;)
  AmiGUS_Interrupt            agp_IntHandler;     // Part's INT sub-handler
  APTR                        agp_IntData;        // Data for INT sub-handler
};

/**
 * Private AmiGUS card representation holding the inner workings
 * of amigus.library,
 * - node structure to be in the list of cards,
 * - public part shared with the clients,
 * - three functional blocks, aka parts, PCM, Wavetable, Codec.
 */
struct AmiGUS_Private {

  struct Node                 agp_Node;           // Can be in a list of cards
  struct AmiGUS               agp_AmiGUS_Public;  // Has a public part!

  struct AmiGUS_Part          agp_PCM;            // PCM functional block
  struct AmiGUS_Part          agp_Wavetable;      // Wavetable functional block
  struct AmiGUS_Part          agp_Codec;          // Codec functional block
};

/*
 * All libraries' base pointers used by the AmiGUS library.
 * Also used to switch between relying on globals or not.
 */
#if defined(BASE_GLOBAL)
  extern struct AmiGUS_Base * AmiGUS_Base;
  extern struct DosLibrary  * DOSBase;
  extern struct Library     * ExpansionBase;
  extern struct Library     * CardResource;
  extern struct ExecBase    * SysBase;
#elif defined(BASE_REDEFINE)
  #define AmiGUS_Base         (base)
  #define CardResource        base->agb_CardResource
  #define DOSBase             base->agb_DOSBase
  #define ExpansionBase       base->agb_ExpansionBase
  #define SysBase             base->agb_SysBase
#endif

#endif /* AMIGUS_PRIVATE_H */
