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

#ifndef AHI_MODES_H
#define AHI_MODES_H

#include "copies.h"

#define AMIGUS_AHI_MIN_MODE ( 0x0ACF0000      )
#define AMIGUS_AHI_MAX_MODE ( 0x0ACF0000 + 20 )

struct PlaybackProperties {
    /** Playback copy function pointer                                       */
    CopyFunctionType    pp_CopyFunction;
    /** AmiGUS hardware register content defining playback sample type       */
    ULONG               pp_HwFormatId;
    /** Translates size of "1" AHI sample to BYTE size of that sample        */
    ULONG               pp_AhiSampleShift;
    /** Byte size of a single (potentially stereo) AmiGUS hardware sample    */
    ULONG               pp_HwSampleSize;
    /** Copy function required alignment encoded as bit mask                 */
    ULONG               pp_AhiBufferMask;
};

struct RecordingProperties {
    /** Recording copy function pointer                                      */
    CopyFunctionType    rp_CopyFunction;
    /** AmiGUS hardware register content defining recording sample type      */
    ULONG               rp_HwFormatId;
    /** AHI sample format identifier                                         */
    ULONG               rp_AhiFormatId;
    /** Translates size of "1" AHI sample to BYTE size of that sample        */
    ULONG               rp_AhiSampleShift;
    /** Recording buffer BYTE size needs to be multiple of these             */
    ULONG               rp_AhiBufferMultiples;
};

extern struct PlaybackProperties PlaybackPropertiesById[];
extern struct RecordingProperties RecordingPropertiesById[];

#endif /* AHI_MODES_H */
