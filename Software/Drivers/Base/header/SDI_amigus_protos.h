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

#ifndef SDI_AMIGUS_PROTOS_H
#define SDI_AMIGUS_PROTOS_H

// TO NEVER BE USED OUTSIDE THE LIBRARY CODE !!!

#include <amigus/amigus.h>
#include <exec/types.h>

#include "amigus_private.h"
#include "SDI_compiler.h"

// Fixes a bug in VBCC - somehow it would not compile the return value of
// AmiGUS_FindCard otherwise.
typedef struct AmiGUS * AmiGUS_PTR;

/******************************************************************************
 * AmiGUS base library interface functions written in SDI_compiler macros,
 * thereby making them compiler agnostic and 
 * adapting them for AmiGUS library internal usage.
 *****************************************************************************/

/**
 * Discovers and iterates over all AmiGUS cards, Zorro2 or PCMCIA.
 * It's interface works like the well-known expansion.library/FindConfigDev().
 *
 * @param card    Old ( struct AmiGUS * ) card returned before to iterate
 *                through available cards, or NULL to get the first one.
 * @param base    Ignore, library magic.
 *
 * @return New ( struct AmiGUS * ) card or NULL if none available.
 */
ASM( AmiGUS_PTR ) SAVEDS AmiGUS_FindCard(
  REG( a0, struct AmiGUS * card ),
  REG( a6, struct AmiGUS_Base * base ));

/**
 * Reserves an AmiGUS card, be it Zorro2 or PCMCIA.
 * See amigus/amigus.h for reference of flags and error codes.
 *
 * You may wanna wrap this in 
 * exec.library/Forbid() + Permit() or even Enable() + Disable() pairs
 * to prevent evil from happening!
 *
 * @param card    Valid ( struct AmiGUS * ) card,
 *                or NULL, w/o chance of success, obviously.
 * @param which   Flags indicating which part of the AmiGUS to reserve,
 *                single AMIGUS_FLAG_* or an OR'ed combination of them.
 * @param owner   An unique pointer to something "owning" the resource for user
 *                code, can be a TaskId, a base pointer, whatever.
 *                User code needs to present it in every call to amigus.library
 *                to prove ownership, that's all.
 *                Yes, 12345 would do, too.
 *                No, NULL would not.
 * @param base    Ignore, library magic.
 * 
 * @return AmiGUS_NoError on full success,
 *         error codes from enum AmiGUS_Errors otherwise, prominently
 *         - AmiGUS_NotFound / 0x0404 / 1028 if card is NULL,
 *         - Some bitmask containing 0x0100 + the part flag if already in use
 */
ASM( ULONG ) SAVEDS AmiGUS_ReserveCard(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base ));

/**
 * Frees ("un-reserves") an AmiGUS card, be it Zorro2 or PCMCIA.
 * User code MUST call amigus.library/AmiGUS_RemoveInterrupt() before
 * freeing up the card!
 * See amigus/amigus.h for reference of flags.
 *
 * You may wanna wrap this in 
 * exec.library/Forbid() + Permit() or even Enable() + Disable() pairs
 * to prevent evil from happening!
 *
 * @param card    Valid ( struct AmiGUS * ) card,
 *                or NULL, w/o chance of success, obviously.
 * @param which   Flags indicating which part of the AmiGUS to free,
 *                single AMIGUS_FLAG_* or an OR'ed combination of them.
 * @param owner   An unique pointer to something "owning" the resource for user
 *                code, can be a TaskId, a base pointer, whatever.
 *                Same as used in AmiGUS_ReserveCard().
 * @param base    Ignore, library magic.
 */
ASM( VOID ) SAVEDS AmiGUS_FreeCard(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base ));

/**
 * Installs an interrupt handler into amigus.library,
 * for a reserved AmiGUS card, be it Zorro2 or PCMCIA.
 * See amigus/amigus.h for reference of flags, error codes,
 * and handler function's signature.
 *
 * No need to wrap this into an exec.library/Forbid() + Permit() pair,
 * as all non-owner requests are rejected.
 * You got race conditions in your own code?
 * Well, that's a different story... maybe fix it where it happens?
 * No? Well, will not crash, likely.
 * But wrapping it into an exec.library/Enable() + Disable() pair will
 * go sideways.
 *
 * @param card    Valid ( struct AmiGUS * ) card,
 *                or NULL, w/o chance of success, obviously.
 * @param which   Flags indicating which part of the AmiGUS to install
 *                the handler for, can be a single AMIGUS_FLAG_*
 *                or an OR'ed combination of them.
 * @param owner   An unique pointer to something "owning" the resource for user
 *                code, can be a TaskId, a base pointer, whatever.
 *                Same as used in AmiGUS_ReserveCard().
 * @param handler User code interrupt handler function,
 *                accepting an APTR to data as per below in d1, returning
 *                1 in d0 if interrupt request handled,
 *                0 otherwise.
 * @param data    Whatever the user code wants to send to the interrupt handler,
 *                maybe a base address? Some pointer to some user data?
 * @param base    Ignore, library magic.
 * 
 * @return AmiGUS_NoError on full success,
 *         error codes from enum AmiGUS_Errors otherwise, prominently
 *         - AmiGUS_NotFound / 0x0404 / 1028 if card is NULL,
 *         - AmiGUS_NotYours / 0x0200 / 512 if card is not owned.
 */
ASM( ULONG ) SAVEDS AmiGUS_InstallInterrupt(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( d2, AmiGUS_Interrupt handler ),
  REG( d3, APTR data ),
  REG( a6, struct AmiGUS_Base * base ));

/**
 * Removes an interrupt handler from amigus.library,
 * for a reserved AmiGUS card, be it Zorro2 or PCMCIA.
 * See amigus/amigus.h for reference of flags and error codes.
 *
 * No need to wrap this into an exec.library/Forbid() + Permit() pair,
 * as all non-owner requests are rejected.
 * You got race conditions in your own code?
 * Well, that's a different story... maybe fix it where it happens?
 * No? Well, will not crash, likely.
 * But wrapping it into an exec.library/Enable() + Disable() pair will
 * go sideways.
 *
 * @param card    Valid ( struct AmiGUS * ) card,
 *                or NULL, w/o chance of success, obviously.
 * @param which   Flags indicating which part of the AmiGUS to install
 *                the handler for, can be a single AMIGUS_FLAG_*
 *                or an OR'ed combination of them.
 * @param owner   An unique pointer to something "owning" the resource for user
 *                code, can be a TaskId, a base pointer, whatever.
 *                Same as used in AmiGUS_ReserveCard() or when installing
 *                the interrupt handler.
 * @param base    Ignore, library magic.
 */
ASM( VOID ) SAVEDS AmiGUS_RemoveInterrupt(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base ));

/******************************************************************************
 * AmiGUS base library interrupt function written in SDI_compiler macros,
 * compiler agnostic and for AmiGUS library internal usage.
 *****************************************************************************/

/**
 * Interrupt handler function, checking the status of all AmiGUS cards
 * found so far and filling their buffers accordingly.
 *
 * @param base Pointer to the driver library's base address.
 *
 * @return 1 if there was at least one card's interrupt pending that was handled,
 *         0 otherwise.
 */
ASM( LONG ) /* __entry for vbcc ? */ SAVEDS INTERRUPT HandleInterrupt (
  REG( a1, struct AmiGUS_Base * base ));

#endif /* SDI_AMIGUS_PROTOS_H */
