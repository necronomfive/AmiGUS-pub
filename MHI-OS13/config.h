/*
  file:   config.h

  author: Henryk Richter <henryk.richter@gmx.net>

*/
#ifndef _INC_CONFIG_H
#define _INC_CONFIG_H

#ifndef _INC_COMPILER_H
#include "compiler.h"
#endif

/*-----------------------------------------------------------------*/
/* some fiddly defines to keep the parser code relatively indendent
   of the i2c_sensorbase stuff, if so desired...
   
   This was added to keep library bases in a well known structure,
   whose contents are referenced for library calls

   If you want to get rid of it, see that the Lib bases of
   SysBase,Utilitybase and DOSBase are global and keep the
   defines below empty
*/
#define BASENAME  example_libbase 
#define BASETYPE  struct example_libbase
#define BASEARG   BASENAME,
#define BASEPROTO ASMR(a6) BASETYPE* ASMREG(a6),
#define BASEDEF   ASMR(a6) BASETYPE*BASENAME ASMREG(a6),
#define BASEPTR  BASETYPE *BASENAME


#endif /* _INC_CONFIG_H */
