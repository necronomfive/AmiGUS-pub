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

#include <stdio.h>
#ifdef NULL
#undef NULL
#endif /* NULL */

#include <proto/exec.h>
#include <proto/intuition.h>

#include <exec/types.h>

#ifndef IDCMP_GADGETUP
#define IDCMP_GADGETUP 0x00000040
#endif /* IDCMP_GADGETUP */

struct Library * ExecBase = NULL;
struct IntuitionBase * IntuitionBase = NULL;

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL response;
  BYTE * message = "Body Text mit wirklich vielen woertern"
  // "und buchstaben in lang und so"
  // "und noch und noch viel viel laenger"
  ;

  struct IntuiText body;
  body.BackPen = 0;
  body.FrontPen = 2;
  body.DrawMode = 0;
  body.ITextFont = NULL;
  body.LeftEdge = 8;
  body.TopEdge = 5;
  body.IText = message;
  body.NextText = NULL;

  struct IntuiText negative;
  negative.BackPen = 0;
  negative.FrontPen = 2;
  negative.DrawMode = 0;
  negative.ITextFont = NULL;
  negative.LeftEdge = 6;
  negative.TopEdge = 3;
  negative.IText = "Negative Text";
  negative.NextText = NULL;

  ExecBase = OpenLibrary( "exec.library", 34 );
  if ( !ExecBase ) {
    return 21;
  }
  
  IntuitionBase = (struct  IntuitionBase * ) OpenLibrary( "intuition.library", 34 );
  if ( !IntuitionBase ) {
    return 22;
  }

  printf( "Libraries opened\n" );

  //response = AutoRequest(
  struct Window * window = BuildSysRequest(
    ( struct Window * ) NULL,
    ( struct IntuiText * ) &body, // body IntuiText
    ( struct IntuiText * ) NULL, // positive IntuiText
    ( struct IntuiText * ) &negative, // negative IntuiText
    ( ULONG ) IDCMP_GADGETUP, // flags
    ( UWORD ) 640, // width
    ( UWORD ) 50 // height
  );
  SetWindowTitles( window, "window title", NULL/*"screen title"*/ );
  Wait( 1L << window->UserPort->mp_SigBit );
  FreeSysRequest( window );

/*
  struct EasyStruct req;
  req.es_StructSize = sizeof( struct EasyStruct );
  req.es_Flags = 0;
  req.es_Title = "asdfasdfasdfasdf";
  req.es_TextFormat = message;
  req.es_GadgetFormat = "text";
  EasyRequest( NULL, &req, NULL, 17, req.es_TextFormat );
*/

  if ( !IntuitionBase ) {

    CloseLibrary( ( struct Library * ) IntuitionBase );
  }

  if ( !ExecBase ) {

    CloseLibrary( ExecBase );
  }
  

  return 0;
}
  