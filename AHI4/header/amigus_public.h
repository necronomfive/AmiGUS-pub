/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_PUBLIC_H
#define AMIGUS_PUBLIC_H

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

#include "amigus_hardware.h"

#define AMIGUS_AHI_AUTHOR           "Christoph `Chritoph` Fassbach"
#define AMIGUS_AHI_COPYRIGHT        "(c) 2024 Christoph Fassbach / LGPL3"
#define AMIGUS_AHI_ANNOTATION       "Thanks to: Oliver Achten (AmiGUS), " \
                                    "Frank Wille (vbcc), Martin Blom (AHI)"
#define AMIGUS_AHI_VERSION          LIBRARY_IDSTRING
#define AMIGUS_AHI_RECORD           FALSE
#define AMIGUS_AHI_FULL_DUPLEX      FALSE

#define AMIGUS_AHI_FIRMWARE_MINIMUM ( ( 2024 << 20 ) /* year   */ \
                                    + (    8 << 16 ) /* month  */ \
                                    + (   17 << 11 ) /* day    */ \
                                    + (   21 <<  6 ) /* hour   */ \
                                    + (   38 <<  0 ) /* minute */ )

#define AMIGUS_AHI_NUM_SAMPLE_RATES 9
#define AMIGUS_AHI_NUM_OUTPUTS      1
#define AMIGUS_AHI_NUM_INPUTS       4

#define AHIDB_AmiGUS_SampleFormat   ( AHIDB_UserBase + 0 )
#define AHIDB_AmiGUS_CopyFunction   ( AHIDB_UserBase + 1 )

#define AMIGUS_MEM_LOG_MARKER        "********************************"   \
                                     " AmiGUS "                           \
                                     "********************************\n"

/* 
 * This is the official part of AmiGUSBase.
 * It has private fields as well. 
 */
struct AmiGUSBase {
  struct Library         agb_LibNode;
  UWORD                  agb_Unused;       /* better alignment */
};

#endif /* AMIGUS_PUBLIC_H */
