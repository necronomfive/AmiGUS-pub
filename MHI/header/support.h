/*
 * This file is part of the mhiamigus.library.
 *
 * mhiamigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiamigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiamigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SUPPORT_H
#define SUPPORT_H

#include <exec/types.h>

/**
 * Initializes a List or a MinList as empty.
 * 
 * Careful, 
 * this involves some cruel casts and trashes everything
 * that is put into it without warnings by the compiler!
 *
 * @param list List or MinList to initialize.
 */
#define NEW_LIST( list ) \
  (( struct List * ) list)->lh_Head = \
    ( struct Node * ) &((( struct List * ) list)->lh_Tail); \
  (( struct List * ) list)->lh_Tail = NULL; \
  (( struct List * ) list)->lh_TailPred = ( struct Node * ) list

/**
 * Iterates with a for-loop over all elements of a List or a MinList.
 * 
 * Careful, 
 * this involves some cruel casts and trashes everything
 * that is put into it without warnings by the compiler!
 *
 * @param list List or MinList to iterate over.
 * @param node Node or MinNode of matching type as in list for iterating.
 * @param node_type Matching type of the Node or MinNode, used for casting.
 */
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

VOID ShowError( STRPTR title, STRPTR message, STRPTR button );

VOID ShowAlert( ULONG alertNum );

#endif /* SUPPORT_H */