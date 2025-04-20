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

#define NEW_LIST( list ) \
  (( struct List * ) list)->lh_Head = \
    ( struct Node * ) &((( struct List * ) list)->lh_Tail); \
  (( struct List * ) list)->lh_Tail = NULL; \
  (( struct List * ) list)->lh_TailPred = ( struct Node * ) list

#define FOR_LIST( list, node, node_type ) \
  for ( node = ( node_type ) (( struct List * ) list)->lh_Head ;\
        ( node_type ) (( struct Node * ) node)->ln_Succ \
          != ( node_type ) (( struct List * ) list)->lh_Tail ;\
        node = ( node_type ) (( struct Node * ) node)->ln_Succ )

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
 * Correctly initializes an empty, new MinList.
 *
 * @param list Pointer to MinList to initialize, will be empty afterwards,
 *             all former content would be orphaned.
 */
VOID NonConflictingNewMinList( struct MinList * list );

VOID ShowError( STRPTR title, STRPTR message, STRPTR button );

VOID ShowAlert( ULONG alertNum );

#endif /* SUPPORT_H */