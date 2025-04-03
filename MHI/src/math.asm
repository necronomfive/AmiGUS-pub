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
 
	xdef __CXM33

	xdef __CXD33
	xdef __divs


	section text,code

* in d0, d1
* out d0 = d0 / d1
__CXM33:
    muls d1,d0
    rts

* in d0, d1
* out d0 = d0 / d1
__CXD33:
__divs:
    divs d1,d0
	rts

	end
