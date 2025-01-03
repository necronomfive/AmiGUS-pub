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

#define AMIGUS_AHI_FIRMWARE_MINIMUM ( ( 2024 << 20 ) /* year   */ \
                                    + (   12 << 16 ) /* month  */ \
                                    + (    8 << 11 ) /* day    */ \
                                    + (   22 <<  6 ) /* hour   */ \
                                    + (   38 <<  0 ) /* minute */ )


#define AHIDB_AmiGUS_PlayCopyFunction   ( AHIDB_UserBase + 0 )
#define AHIDB_AmiGUS_PlayHwSampleId     ( AHIDB_UserBase + 1 )
#define AHIDB_AmiGUS_RecCopyFunction    ( AHIDB_UserBase + 2 )
#define AHIDB_AmiGUS_RecHwSampleId      ( AHIDB_UserBase + 3 )

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
