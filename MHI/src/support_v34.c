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

#include <proto/exec.h>
#include <proto/intuition.h>

#include <exec/types.h>

#include "amigus_mhi.h"
#include "debug.h"

#ifndef IDCMP_GADGETUP
#define IDCMP_GADGETUP 0x00000040
#endif /* IDCMP_GADGETUP */

#ifdef LIBRARY_VERSION
/* Patches to make this compile in NDK1.3 */
#define Alert13Compat( x )        Alert( x, NULL )
#else
#define Alert13Compat( x )        Alert( x )
#endif

VOID ShowError( STRPTR title, STRPTR message, STRPTR button ) {

  struct IntuiText body;
  struct IntuiText negative;
  struct Window * window;

  body.BackPen = 0;
  body.FrontPen = 2;
  body.DrawMode = 0;
  body.ITextFont = NULL;
  body.LeftEdge = 8;
  body.TopEdge = 5;
  body.IText = message;
  body.NextText = NULL;

  negative.BackPen = 0;
  negative.FrontPen = 2;
  negative.DrawMode = 0;
  negative.ITextFont = NULL;
  negative.LeftEdge = 6;
  negative.TopEdge = 3;
  negative.IText = button;
  negative.NextText = NULL;

  window = BuildSysRequest(
    ( struct Window * ) NULL,         // parent Window
    ( struct IntuiText * ) &body,     // body IntuiText
    ( struct IntuiText * ) NULL,      // positive IntuiText
    ( struct IntuiText * ) &negative, // negative IntuiText
    ( ULONG ) IDCMP_GADGETUP,         // flags
    ( UWORD ) 640,                    // width
    ( UWORD ) 50                      // height
  );
  SetWindowTitles( window, title, NULL /*"screen title"*/ );
  Wait( 1L << window->UserPort->mp_SigBit );
  FreeSysRequest( window );
}

VOID ShowAlert( ULONG alertNum ) {

  Alert13Compat( alertNum );
}
