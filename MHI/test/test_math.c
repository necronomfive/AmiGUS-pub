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

#include <exec/types.h>
#include <libraries/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "math.h"

struct ExecBase          * SysBase;
struct DosLibrary        * DOSBase;

int main(int argc, char const *argv[]) {

  SysBase = ( * (( struct ExecBase ** ) 4 ));
  DOSBase = ( struct DosLibrary * ) OpenLibrary( "dos.library", 0L );
  Printf( "12 * 3 = %ld\n", _CXM33( 12, 3 ));
  Printf( "12 / 3 = %ld\n", _divs( 12, 3 ));
  CloseLibrary(( struct Library * ) DOSBase );
}
