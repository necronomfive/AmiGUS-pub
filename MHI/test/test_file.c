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

/*
vc +kick13 -Iheader test/test_file.c -o test/test_file
*/

#include <stdio.h>
#ifdef NULL
#undef NULL
#endif /* NULL */
 
#include <proto/dos.h>
#include <proto/exec.h>
 
#include <libraries/dos.h>

#include <exec/types.h>

struct Library * ExecBase = NULL;
struct Library * IntuitionBase = NULL;
 
/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  UBYTE buffer[] = "blabla\n";
  UBYTE fileName[] = "ram:test.txt";

  ExecBase = OpenLibrary( "exec.library", 34 );
  if ( !ExecBase ) {
    printf( "exec failed\n" );
    return 21;
  }
  DOSBase = ( struct DosLibrary * ) OpenLibrary( "dos.library", 34 );
  if ( !DOSBase ) {
    printf( "dos failed\n" );
    return 22;
  }

  printf( "Printing \"%s\" into file %s\n", buffer, fileName );
  BPTR file = Open( fileName, MODE_NEWFILE );
  Write( file, buffer, sizeof( buffer ));
  Close( file );

#if 1
  if ( DOSBase ) {
    CloseLibrary( ( struct Library * ) DOSBase );
    // crashes, I guess vbcc kick1.3 startup code closes it
    // DOSBase = NULL;
  }
#endif
  if ( ExecBase ) {
    CloseLibrary( ExecBase );
    ExecBase = NULL;
  }
  return 0;
}
  