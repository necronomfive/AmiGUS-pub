/*
 * This file is part of the mhiamigus.library.
 *
 * mhiamigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiamigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiamigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include <proto/exec.h>

#include "amigus_mhi.h"
#include "support.h"

struct AmiGUS_MHI        * AmiGUS_MHI_Base   = NULL;

BOOL testListCount( struct MinList * list, LONG exp ) {

  BOOL failed = FALSE;
  LONG count = 0;

  const struct MinNode * tail =
    ( const struct MinNode * )
      list->mlh_Tail;
  struct AmiGUS_MHI_Handle * currentHandle =
    ( struct AmiGUS_MHI_Handle * )
      list->mlh_Head;
  while ( tail != currentHandle->agch_Node.mln_Succ ) {

    ++count;
    printf( "tick %ld - node 0x%08lx\n", count, ( LONG ) currentHandle );
    currentHandle = ( struct AmiGUS_MHI_Handle * ) currentHandle->agch_Node.mln_Succ;
  }
  failed |= ( exp != count );

  return failed;
}

BOOL testListMacroCount( struct MinList * list, LONG exp ) {

  BOOL failed = FALSE;
  LONG count = 0;

  struct AmiGUS_MHI_Handle * node;

  FOR_LIST( list, node, struct AmiGUS_MHI_Handle * ) {

    ++count;
    printf( "tick %ld - node 0x%08lx\n", count, ( LONG ) node );
  }
  failed |= ( exp != count );

  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;
  struct MinList list;
  struct AmiGUS_MHI_Handle handleA;
  struct AmiGUS_MHI_Handle handleB;
  struct AmiGUS_MHI_Handle handleC;
  struct AmiGUS_MHI_Handle handleD;
  struct AmiGUS_MHI_Handle handleE;

  NEW_LIST( &list );

  failed |= testListCount( &list, 0 );
  failed |= testListMacroCount( &list, 0 );
  printf( "\n" );
  AddTail(( struct List * ) &list,
          ( struct Node * ) &handleA );
  failed |= testListCount( &list, 1 );
  failed |= testListMacroCount( &list, 1 );
  printf( "\n" );
  AddTail(( struct List * ) &list,
          ( struct Node * ) &handleB );
  AddTail(( struct List * ) &list,
          ( struct Node * ) &handleC );
  AddTail(( struct List * ) &list,
          ( struct Node * ) &handleD );
  AddTail(( struct List * ) &list,
          ( struct Node * ) &handleE );
  failed |= testListCount( &list, 5 );
  failed |= testListMacroCount( &list, 5 );
  printf( "\n" );
  return ( failed ) ? 15 : 0;
}