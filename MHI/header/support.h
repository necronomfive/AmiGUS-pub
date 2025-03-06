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

#ifndef SUPPORT_H
#define SUPPORT_H

#include <exec/types.h>

/**
 * Displays an error message in a requester.
 *
 * Displays an error message, showing the error code and
 * a error message defined in errors[]. If a code can not
 * be resolved, the EUnknownError text is displayed.
 *
 * @param aBase Library base pointer used for calling functions
 *              of other libraries from it.
 * @param aError Error Id to display error message for.
 */
VOID DisplayError( ULONG aError );

/**
 * Prints some time info.
 */
VOID LogTicks( UBYTE bitmask );

/**
 * Waits for some milli seconds... almost,
 * assuming 1 second has 1024 milliseconds.
 */
VOID Sleep( UWORD pseudomillis );

/**
 * Correctly initializes an empty, new MinList.
 *
 * @param list Pointer to MinList to initialize, will be empty afterwards,
 *             all former content would be orphaned.
 */
VOID NonConflictingNewMinList( struct MinList * list );

#endif /* SUPPORT_H */
