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

#ifndef COMPILER_EXTRAS_H
#define  COMPILER_EXTRAS_H

#include <clib/compiler-specific.h>

/**
 * Usage: x = GET_REG(REG_D1)
 */
#if defined(__VBCC__)

__reg("d1") ULONG __GET_REG_D1()="\t";
#define GET_REG(reg) __GET_ ## reg()

#elif defined(__SASC)

#define GET_REG getreg

#define getreg __builtin_getreg
extern long getreg(int);

#define REG_D1 1

#endif

/**
 * USAGE: INLINE LONG foo( VOID );
 */
#if defined(__VBCC__)

#undef INLINE
#define INLINE static 

#elif defined(__SASC)

#define INLINE static __inline

#endif

/**
 * USAGE: LONG positionOfFieldInStruct = OFFSET( structName, structField );
 * 
 * Multi-Compiler Version of exec/initializers.h, for 1.3 compatibility et al..
 */
#ifndef OFFSET

#if defined(__VBCC__)

#define OFFSET(p,m) __offsetof(struct p,m)

#else

#define OFFSET(structName, structEntry) \
    ((char *)(&(((struct structName *)0)->structEntry))-(char *)0)

#endif

#endif /* OFFSET */

/**
 * USAGE: TODO: ?
 */
#if defined(__VBCC__)

#define AMIGA_INTERRUPT __amigainterrupt

#elif defined(__SASC)

#define AMIGA_INTERRUPT __interrupt

#endif


#endif /* COMPILER_EXTRAS_H */
