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

#ifndef MATH_H
#define MATH_H

#include "SDI_compiler.h"

/**
 * Math glue function for SAS/C, used e.g. in MHI EQ.
 * Needed as SAS/C does not want to understand a
 * 16bit x 16bit multiplication is enough in this particular case.
 * And as we are not linking any standard libraries here...
 *
 * @param a One factor.
 * @param b Second factor.
 *
 * @return Product a * b.
 */
ASM( LONG ) SAVEDS _CXM33( REG( d0, WORD a ), REG( d1, WORD b ));

/**
 * Math glue function for SAS/C + VBCC, used e.g. in MHI EQ.
 * Needed as both do not want to understand a
 * 32bit / 16bit division is enough in this particular case.
 * And as we are not linking any standard libraries here...
 *
 * @param a Dividend.
 * @param b Divisor.
 *
 * @return Quotient a / b.
 */
ASM( LONG ) _divs( REG( d0, LONG a ), REG( d1, LONG b ));

#endif /* MATH_H */
