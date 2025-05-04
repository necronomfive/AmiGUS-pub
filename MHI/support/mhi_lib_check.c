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

// vc +kick13 -I../include -I../header test_library.c -o test_library

#include <stdio.h>
#include <string.h>

#ifdef NULL
#undef NULL
#endif

#include <proto/exec.h>
#include <proto/mhi.h>
#include <libraries/mhi.h>

#define SDI_MHI_PROTOS_H

#include "amigus_mhi.h"

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/

// inlines created by
// fd2pragma SPECIAL 70 CLIB /clib/mhi_protos.h INFILE mhi_lib.fd TO /INLINE/
#ifdef __VBCC__
#ifndef _VBCCINLINE_MHI_H
#define _VBCCINLINE_MHI_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

APTR __MHIAllocDecoder(__reg("a6") struct Library *, __reg("a0") struct Task * task, __reg("d0") ULONG mhisignal)="\tjsr\t-30(a6)";
#define MHIAllocDecoder(task, mhisignal) __MHIAllocDecoder(MHIBase, (task), (mhisignal))

VOID __MHIFreeDecoder(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-36(a6)";
#define MHIFreeDecoder(handle) __MHIFreeDecoder(MHIBase, (handle))

BOOL __MHIQueueBuffer(__reg("a6") struct Library *, __reg("a3") APTR handle, __reg("a0") APTR buffer, __reg("d0") ULONG size)="\tjsr\t-42(a6)";
#define MHIQueueBuffer(handle, buffer, size) __MHIQueueBuffer(MHIBase, (handle), (buffer), (size))

APTR __MHIGetEmpty(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-48(a6)";
#define MHIGetEmpty(handle) __MHIGetEmpty(MHIBase, (handle))

UBYTE __MHIGetStatus(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-54(a6)";
#define MHIGetStatus(handle) __MHIGetStatus(MHIBase, (handle))

VOID __MHIPlay(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-60(a6)";
#define MHIPlay(handle) __MHIPlay(MHIBase, (handle))

VOID __MHIStop(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-66(a6)";
#define MHIStop(handle) __MHIStop(MHIBase, (handle))

VOID __MHIPause(__reg("a6") struct Library *, __reg("a3") APTR handle)="\tjsr\t-72(a6)";
#define MHIPause(handle) __MHIPause(MHIBase, (handle))

ULONG __MHIQuery(__reg("a6") struct Library *, __reg("d1") ULONG query)="\tjsr\t-78(a6)";
#define MHIQuery(query) __MHIQuery(MHIBase, (query))

VOID __MHISetParam(__reg("a6") struct Library *, __reg("a3") APTR handle, __reg("d0") UWORD param, __reg("d1") ULONG value)="\tjsr\t-84(a6)";
#define MHISetParam(handle, param, value) __MHISetParam(MHIBase, (handle), (param), (value))

#endif /*  _VBCCINLINE_MHI_H  */
#endif /* __VBCC__ */

#ifdef __SASC

#pragma libcall MHIBase MHIAllocDecoder 1e 0802
#pragma libcall MHIBase MHIFreeDecoder 24 B01
#pragma libcall MHIBase MHIQueueBuffer 2a 08B03
#pragma libcall MHIBase MHIGetEmpty 30 B01
#pragma libcall MHIBase MHIGetStatus 36 B01
#pragma libcall MHIBase MHIPlay 3c B01
#pragma libcall MHIBase MHIStop 42 B01
#pragma libcall MHIBase MHIPause 48 B01
#pragma libcall MHIBase MHIQuery 4e 101
#pragma libcall MHIBase MHISetParam 54 10B03

#endif

struct Library * MHIBase;

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testQuery( VOID ) {

  BOOL failed = FALSE;
  BYTE * exp;
  BYTE * out;

  out = ( BYTE * ) MHIQuery( MHIQ_DECODER_NAME );
  exp = AMIGUS_MHI_DECODER;
  failed |= strcmp( exp, out );
  printf( "Name    - %s\t- exp %s act %s\n",
          failed ? "failed" : "OK", exp, out );

  out = ( BYTE * ) MHIQuery( MHIQ_AUTHOR );
  exp = AMIGUS_MHI_AUTHOR    " \n"
        AMIGUS_MHI_COPYRIGHT " \n" 
        AMIGUS_MHI_ANNOTATION;
  failed |= strcmp( exp, out );
  printf( "Name    - %s\t- exp \n%s\n act \n%s\n",
          failed ? "failed" : "OK", exp, out );

  out = ( BYTE * ) MHIQuery( MHIQ_DECODER_VERSION );
  exp = AMIGUS_MHI_VERSION;
  failed |= strcmp( exp, out );
  printf( "Name    - %s\t- exp %s act %s\n",
          failed ? "failed" : "OK", exp, out );

  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;
  STRPTR libraryName = "mhiamigus.library";
  STRPTR attempt2 = "libs:mhi/mhiamigus.library";

  MHIBase = OpenLibrary( libraryName, 0 );
  if ( !MHIBase ) {

    printf( "Opening %s failed!\n", libraryName );
    libraryName = attempt2;
    MHIBase = OpenLibrary( libraryName, 0 );
    if ( !MHIBase ) {

      printf( "Opening %s failed!\n", libraryName );
      return 20;
    }
  }
  printf( "%s opened\n", libraryName );

  failed |= testQuery();

  CloseLibrary( MHIBase );
  printf( "%s closed\n", libraryName );

  return ( failed ) ? 15 : 0;
}
