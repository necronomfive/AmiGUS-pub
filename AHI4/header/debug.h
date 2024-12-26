/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <exec/types.h>

// For Sashimi, we need to use Kick1.2 methods here...
// RawIOInit()              -504
// RawMayGetChar()          -510
// RawPutChar(char)(d0)     -516

#if defined (__VBCC__)

VOID __RawIOInit(__reg("a6") void *)="\tjsr\t-504(a6)";
#define RawIOInit() __RawIOInit(SysBase)

LONG __RawMayGetChar(__reg("a6") void *)="\tjsr\t-510(a6)";
#define RawMayGetChar() __RawMayGetChar(SysBase)

VOID __RawPutChar(__reg("a6") void *, __reg("d0") BYTE putCh)="\tjsr\t-516(a6)";
#define RawPutChar(putCh) __RawPutChar(SysBase, (putCh))

#elif defined (__SASC)

VOID RawIOInit( VOID );
LONG RawMayGetChar( VOID );
VOID RawPutChar(BYTE putCh);

#pragma libcall SysBase RawIOInit     1f8 0
#pragma libcall SysBase RawMayGetChar 1fe 00
#pragma libcall SysBase RawPutChar    204 01

#endif

// //////////////////

/*
 * Remember: 
 * All of these will need double ((...)) to workaround a SAS/C insufficiency!
 */

#define USE_MEM_LOGGING
#if defined (USE_SERIAL_LOGGING)

// LOG_INT(X) would deadlock
#define LOG_V(X) debug_kprintf X
#define LOG_D(X) debug_kprintf X
#define LOG_I(X) debug_kprintf X
#define LOG_W(X) debug_kprintf X
#define LOG_E(X) debug_kprintf X

#elif defined (USE_FILE_LOGGING)

// LOG_INT(X) would crash
#define LOG_V(X) debug_fprintf X
#define LOG_D(X) debug_fprintf X
#define LOG_I(X) debug_fprintf X
#define LOG_W(X) debug_fprintf X
#define LOG_E(X) debug_fprintf X

#elif defined (USE_MEM_LOGGING)

#define LOG_INT(X) debug_mprintf X
#define LOG_V(X)   debug_mprintf X
#define LOG_D(X)   debug_mprintf X
#define LOG_I(X)   debug_mprintf X
#define LOG_W(X)   debug_mprintf X
#define LOG_E(X)   debug_mprintf X

#endif

/* Used to disable loglevels without warnings! */
#ifndef LOG_INT
#define LOG_INT(X)
#endif
#ifndef LOG_V
#define LOG_V(X)
#endif
#ifndef LOG_D
#define LOG_D(X)
#endif
#ifndef LOG_I
#define LOG_I(X)
#endif
#ifndef LOG_W
#define LOG_W(X)
#endif
#ifndef LOG_E
#define LOG_E(X)
#endif

/*
 * Remember: 
 * Whatever parameter you pass in here will ALWAYS be promoted to LONG!
 */
void debug_kprintf(STRPTR format, ...);
void debug_fprintf(STRPTR format, ...);
void debug_mprintf(STRPTR format, ...);

#endif /* DEBUG_H */
