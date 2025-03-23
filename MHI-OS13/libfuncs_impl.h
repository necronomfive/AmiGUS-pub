/*
  file:   libfuncs_impl.h

  author: Henryk Richter <henryk.richter@gmx.net>

*/
#ifndef _INC_LIBFUNCS_IMPL_H
#define _INC_LIBFUNCS_IMPL_H

#ifndef DOS_DOS_H
#include <libraries/dos.h>
#endif
#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif
#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif

#include "compiler.h"

#ifdef _MYLIBRARY_FUNCS_INTERNAL
/* redirect Library pointers */
#define SysBase example_libbase->sysbase
#define UtilityBase example_libbase->utilitylib
#define DOSBase example_libbase->doslib
/*#define MathIeeeSingBasBase i2c_sensorbase->mathieeesingbas*/
/*#define TimerBase (struct Library *)i2c_sensorbase->timer.tr_node.io_Device*/
#endif

/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* structures                                                         */
/* ------------------------------------------------------------------ */
struct example_libbase {
	struct Library libnode;
	BPTR   seglist;
	struct SignalSemaphore LockSemaphore;
	/* */
	struct Library *sysbase; /* */
	struct Library *doslib;
	struct Library *utilitylib;
};
#define EXAMPLE_LIBBASE_DECL


/* ------------------------------------------------------------------ */
/* Functions                                                          */
/* ------------------------------------------------------------------ */
/*
*/
ASM SAVEDS LONG            LibNull(           void                                        );
ASM SAVEDS struct Library *LibInit(           ASMR(d0) struct example_libbase * ASMREG(d0),
                                       ASMR(a0) BPTR                     ASMREG(a0),
                                       ASMR(a6) struct Library *         ASMREG(a6) );
ASM SAVEDS struct Library *LibOpen(    ASMR(a6) struct example_libbase * ASMREG(a6) );			   
ASM SAVEDS BPTR                   LibClose(   ASMR(a6) struct example_libbase * ASMREG(a6) );			   
ASM SAVEDS BPTR                   LibExpunge( ASMR(a6) struct example_libbase * ASMREG(a6) );

#endif /* _INC_LIBFUNCS_IMPL_H */
