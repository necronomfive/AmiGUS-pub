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

#ifndef DEBUG_H
#define DEBUG_H

#include <exec/types.h>

/******************************************************************************
 * Exec stubs.
 *****************************************************************************/

/*
 * For Sashimi, need to use Kick1.2 Exec functions here...
 * RawIOInit()              -504    // Set serial port's baud rate
 * RawMayGetChar()          -510    // Fetch data from serial port
 * RawPutChar(char)(d0)     -516    // Put 1 char to serial port
 * but as they have been removed in "later" NDKs, redefining them here.
 * Not funny... but SAS/C et al do the same and as not linking with standard
 * libraries...
 */

#if defined (__VBCC__)
/* VBCC compatible inlines */

VOID __RawIOInit( __reg( "a6" ) VOID * ) = "\tjsr\t-504(a6)";
#define RawIOInit() __RawIOInit( SysBase )

LONG __RawMayGetChar( __reg( "a6" ) VOID * ) = "\tjsr\t-510(a6)";
#define RawMayGetChar() __RawMayGetChar( SysBase )

VOID __RawPutChar( __reg( "a6" ) VOID *, __reg( "d0" ) BYTE putCh ) \
  = "\tjsr\t-516(a6)";
#define RawPutChar( putCh ) __RawPutChar( SysBase, ( putCh ))

#elif defined (__SASC)

/* SAS/C compatible pragmas */

VOID RawIOInit( VOID );
LONG RawMayGetChar( VOID );
VOID RawPutChar( BYTE putCh );

#pragma libcall SysBase RawIOInit     1f8 0
#pragma libcall SysBase RawMayGetChar 1fe 00
#pragma libcall SysBase RawPutChar    204 01

#endif

/******************************************************************************
 * Debug helper functions.
 *****************************************************************************/

/*
 * Logging macros for different log levels,
 * - LOG_INT errupt - logs details of interrupt handler methods,
 *                    may impact performance, only available logging to memory!
 * - LOG_V erbose - even more chatty details than debug ;)
 * - LOG_D ebug - meant for debugging purposes
 * - LOG_I nfo - contains information that may even make sense to users
 * - LOG_W arning - something has been odd but recoverable
 * - LOG_E rror - non-recoverable something happened
 *
 * Remember: 
 * - All of these require double ((...)) to workaround a SAS/C insufficiency!
 * - All of them accept an exec's Printf style format string
 *   plus variable parameters
 * - Whatever parameter you pass in here will ALWAYS be promoted to LONG!
 */

#if defined (SER_LOG)

// LOG_INT(X) would deadlock
// #define LOG_V(X) debug_kprintf X
#define LOG_D(X) debug_kprintf X
#define LOG_I(X) debug_kprintf X
#define LOG_W(X) debug_kprintf X
#define LOG_E(X) debug_kprintf X

#elif defined (FILE_LOG)

// LOG_INT(X) would crash
#define LOG_V(X) debug_fprintf X
#define LOG_D(X) debug_fprintf X
#define LOG_I(X) debug_fprintf X
#define LOG_W(X) debug_fprintf X
#define LOG_E(X) debug_fprintf X

#elif defined (MEM_LOG)

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

/**
 * Logging function used by the macros above, logging to serial port.
 * Do not use directly, use the macros!
 *
 * @param format Exec Printf style format followed by all parameters.
 */
void debug_kprintf(STRPTR format, ...);

/**
 * Logging function used by the macros above, logging to a file.
 * Do not use directly, use the macros!
 *
 * @param format Exec Printf style format followed by all parameters.
 */
void debug_fprintf(STRPTR format, ...);

/**
 * Logging functions used by the macros above, logging some area in memory.
 * Do not use directly, use the macros!
 *
 * @param format Exec Printf style format followed by all parameters.
 */
void debug_mprintf(STRPTR format, ...);

#endif /* DEBUG_H */
