/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_AHI_MODES_H
#define AMIGUS_AHI_MODES_H

#include "copies.h"

/** Start and end of the AHI modes in audiomodes file. */
#define AMIGUS_AHI_MIN_MODE ( 0x0ACF0000      )
/** End of the AHI modes in audiomodes file. */
#define AMIGUS_AHI_MAX_MODE ( AMIGUS_AHI_MIN_MODE + 20 )

/**
 * Combines all the playback properties per AHI audio mode together,
 * that would be nasty to have in the AHI audiomode file,
 * e.g. function pointer or firmware implementation dependencies,
 * as audiomode changes require an Amiga reboot otherwise.
 */
struct PlaybackProperties {
    /** Playback copy function to copy AHI buffer to AmiGUS PCM FiFo.        */
    CopyFunctionType    pp_CopyFunction;
    /** AmiGUS PCM Playback Sample Format as understood in amigus_hardware.h,*
     *  obviously needs to understand the output of the copy function.       */
    ULONG               pp_HwFormatId;
    /** Byte size of mono / stereo samples in AmiGUS format as expected by   *
     *  the hardware, respectively AmiGUS PCM Playback Sample Format.        */
    ULONG               pp_HwSampleSize;
    /** Number of shifts to translate a single AHI Sample to its BYTE size.  *
     *  Example: Stereo HiFi has 2 x 32 bit = 2 x 4 bytes = 8 bytes = 1<<3.  */
    ULONG               pp_AhiSampleShift;
    /** Buffer alignment as required by the selected copy function encoded   *
     *  as bit mask.                                                         */
    ULONG               pp_AhiBufferMask;
};

/**
 * Combines all the recording properties per AHI audio mode together,
 * that would be nasty to have in the AHI audiomode file,
 * e.g. function pointer or firmware implementation dependencies,
 * as audiomode changes require an Amiga reboot otherwise.
 */
struct RecordingProperties {
    /** Recording copy function to copy AmiGUS PCM FiFo to AHI buffer.       */
    CopyFunctionType    rp_CopyFunction;
    /** AmiGUS PCM Recording Sample Format as understood in                  *
     *  amigus_hardware.h, obviously needs to deliver the input of           *
     *  the copy function and provide the correct AHI sample type as output. */
    ULONG               rp_HwFormatId;
    /** AHI sample type output when recording.                               */
    ULONG               rp_AhiFormatId;
    /** Number of shifts to translate a single AHI Sample to its BYTE size.  *
     *  Example: Stereo 16bit has 2 x 16 bit = 2 x 2 bytes = 4 bytes = 1<<2. */
    ULONG               rp_AhiSampleShift;
    /** Number of BYTEs output per single copy function call, depends on the *
     *  AmiGUS hardware sample format and mainly of the requirement to read  *
     *  only LONGs from AmiGUS. The recording buffers' BYTE size needs to be *
     *  multiples of these.                                                  */
    ULONG               rp_AhiBufferMultiples;
    /** Number of BYTEs input per single copy function call,                 *
     *  only depends on the CopyFunction - but that is defined by the        *
     *  requirements of hardware and AHI in the end.                         */
    ULONG               rp_CopyFunctionInputSize;
};

/**
 * Array of playback properties as per above,
 * to be addressed as per the offset from AMIGUS_AHI_MIN_MODE,
 * i.e. the "vertical" order of the entries has to be the same as in 
 * the audiomodes IFF file.
 */
extern struct PlaybackProperties PlaybackPropertiesById[];

/**
 * Array of recording properties as per above,
 * to be addressed as per the offset from AMIGUS_AHI_MIN_MODE,
 * i.e. the "vertical" order of the entries has to be the same as in 
 * the audiomodes IFF file.
 */
extern struct RecordingProperties RecordingPropertiesById[];

#endif /* AMIGUS_AHI_MODES_H */
