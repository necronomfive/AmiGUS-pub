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

#include <proto/exec.h>

#include "amigus_private.h"
#include "debug.h"

#define NOT_USE_RawPutCharC
#ifdef USE_RawPutCharC

// Comes with a tiny performance impact,
// so... rather not do it.
VOID RawPutCharC(__reg("d0") BYTE putCh) {
    RawPutChar(putCh);
}

VOID debug_kprintf(STRPTR format, ...) {
    RawIOInit();
    RawDoFmt(format, 0, (VOID (*)())&RawPutCharC,0);
    RawMayGetChar();
} 

#else

VOID debug_kvprintf(STRPTR format, APTR vargs) {
  RawIOInit();
  // Ugly like hell, but...
  RawDoFmt(
    format,
    vargs,
    /*
    /----------+----------------------> Cast to void(*)() function pointer
    |          |  /----+--------------> Address math in LONG works better
    |          |  |    | /-----+------> Exec kick1.2 function using SysBase
    |          |  |    | |     | /--+-> RawPutChar is -516 (see above)
    |          |  |    | |     | |  |   */
    (VOID (*)()) ((LONG) SysBase -516),
    0);
  RawMayGetChar();
}

VOID debug_kprintf(STRPTR format, ...) {
  debug_kvprintf(
    format,
    /*
    /----+--------------------> Cast to required APTR 
    |    |  /----+------------> Nice math in LONGs to make it work 
    |    |  |    | /-----+----> STRPTR, address of format, first argument
    |    |  |    | |     | /+-> 4 bytes later, the next argument follows
    |    |  |    | |     | ||   */
    (APTR) ((LONG) &format +4));
}

#endif
