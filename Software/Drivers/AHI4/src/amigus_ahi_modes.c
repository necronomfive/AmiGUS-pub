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

#if defined (__VBCC__)
#pragma dontwarn 61
#endif

#include <devices/ahi.h>

#if defined (__VBCC__)
#pragma popwarn
#endif

#include "amigus_ahi_modes.h"
#include "amigus_hardware.h"

struct PlaybackProperties PlaybackPropertiesById[] = {
  /* 8bit mono */
  { &PlaybackCopy16to8,  AMIGUS_PCM_S_PLAY_MONO_8BIT,    1, 1, 0xffFFffF8 },
  /* 8bit stereo */
  { &PlaybackCopy16to8,  AMIGUS_PCM_S_PLAY_STEREO_8BIT,  2, 2, 0xffFFffF8 },
  /* 8bit stereo++ */
  { &PlaybackCopy16to8,  AMIGUS_PCM_S_PLAY_STEREO_8BIT,  2, 2, 0xffFFffF8 },
  /* 16bit mono */
  { &PlaybackCopy16to16, AMIGUS_PCM_S_PLAY_MONO_16BIT,   2, 1, 0xffFFffFC },
  /* 16bit stereo */
  { &PlaybackCopy16to16, AMIGUS_PCM_S_PLAY_STEREO_16BIT, 4, 2, 0xffFFffFC },
  /* 16bit stereo++ */
  { &PlaybackCopy16to16, AMIGUS_PCM_S_PLAY_STEREO_16BIT, 4, 2, 0xffFFffFC },
  /* Fast 8 bit mono */
  { &PlaybackCopy16to8,  AMIGUS_PCM_S_PLAY_MONO_8BIT,    1, 1, 0xffFFffF8 },
  /* Fast 8 bit stereo */
  { &PlaybackCopy16to8,  AMIGUS_PCM_S_PLAY_STEREO_8BIT,  2, 2, 0xffFFffF8 },
  /* Fast 8 bit stereo++ */
  { &PlaybackCopy16to8,  AMIGUS_PCM_S_PLAY_STEREO_8BIT,  2, 2, 0xffFFffF8 },
  /* Fast 16 bit mono */
  { &PlaybackCopy16to16, AMIGUS_PCM_S_PLAY_MONO_16BIT,   2, 1, 0xffFFffFC },
  /* Fast 16 bit stereo */
  { &PlaybackCopy16to16, AMIGUS_PCM_S_PLAY_STEREO_16BIT, 4, 2, 0xffFFffFC },
  /* Fast 16 bit stereo++ */
  { &PlaybackCopy16to16, AMIGUS_PCM_S_PLAY_STEREO_16BIT, 4, 2, 0xffFFffFC },
  /* HiFi 8 bit mono */
  { &PlaybackCopy32to8,  AMIGUS_PCM_S_PLAY_MONO_8BIT,    1, 2, 0xffFFffF0 },
  /* HiFi 8 bit stereo */
  { &PlaybackCopy32to8,  AMIGUS_PCM_S_PLAY_STEREO_8BIT,  2, 3, 0xffFFffF0 },
  /* HiFi 8 bit stereo++ */
  { &PlaybackCopy32to8,  AMIGUS_PCM_S_PLAY_STEREO_8BIT,  2, 3, 0xffFFffF0 },
  /* HiFi 16 bit mono */
  { &PlaybackCopy32to16, AMIGUS_PCM_S_PLAY_MONO_16BIT,   2, 2, 0xffFFffF8 },
  /* HiFi 16 bit stereo */
  { &PlaybackCopy32to16, AMIGUS_PCM_S_PLAY_STEREO_16BIT, 4, 3, 0xffFFffF8 },
  /* HiFi 16 bit stereo++ */
  { &PlaybackCopy32to16, AMIGUS_PCM_S_PLAY_STEREO_16BIT, 4, 3, 0xffFFffF8 },
  /* HiFi 24 bit mono */
  { &PlaybackCopy32to24, AMIGUS_PCM_S_PLAY_MONO_24BIT,   3, 2, 0xffFFffF0 },
  /* HiFi 24 bit stereo */
  { &PlaybackCopy32to24, AMIGUS_PCM_S_PLAY_STEREO_24BIT, 6, 3, 0xffFFffF0 },
  /* HiFi 24 bit stereo++ */
  { &PlaybackCopy32to24, AMIGUS_PCM_S_PLAY_STEREO_24BIT, 6, 3, 0xffFFffF0 }
/*  ^1                   ^2                              ^3 ^4 ^5
 *
 * Note ^1:
 *   Playback copy function to copy AHI buffer to AmiGUS PCM FiFo.
 * Note ^2:
 *   AmiGUS PCM Playback Sample Format as understood in amigus_hardware.h,
 *   obviously needs to understand the output of the copy function.
 * Note ^3:
 *   Byte size of mono / stereo samples in AmiGUS format as expected by the
 *   hardware, respectively AmiGUS PCM Playback Sample Format.
 * Note ^4:
 *   Number of shifts to translate a single AHI Sample to its BYTE size.
 *   Example: Stereo HiFi has 2 x 32 bit = 2 x 4 bytes = 8 bytes = 1<<3.
 * Note ^5:
 *   Buffer alignment as required by the selected copy function encoded as bit mask.
  */
};

struct RecordingProperties RecordingPropertiesById[] = {
  /* 8bit mono */
  { &RecordingCopy8Mto16S,  AMIGUS_PCM_S_REC_MONO_8BIT,    AHIST_S16S, 2, 16,  4 },
  /* 8bit stereo */
  { &RecordingCopy8Sto16S,  AMIGUS_PCM_S_REC_STEREO_8BIT,  AHIST_S16S, 2,  8,  4 },
  /* 8bit stereo++ */
  { &RecordingCopy8Sto16S,  AMIGUS_PCM_S_REC_STEREO_8BIT,  AHIST_S16S, 2,  8,  4 },
  /* 16bit mono */
  { &RecordingCopy16Mto16S, AMIGUS_PCM_S_REC_MONO_16BIT,   AHIST_S16S, 2,  8,  4 },
  /* 16bit stereo */
  { &RecordingCopy16Sto16S, AMIGUS_PCM_S_REC_STEREO_16BIT, AHIST_S16S, 2,  4,  4 },
  /* 16bit stereo++ */
  { &RecordingCopy16Sto16S, AMIGUS_PCM_S_REC_STEREO_16BIT, AHIST_S16S, 2,  4,  4 },
  /* Fast 8 bit mono */
  { &RecordingCopy8Mto16S,  AMIGUS_PCM_S_REC_MONO_8BIT,    AHIST_S16S, 2, 16,  4 },
  /* Fast 8 bit stereo */
  { &RecordingCopy8Sto16S,  AMIGUS_PCM_S_REC_STEREO_8BIT,  AHIST_S16S, 2,  8,  4 },
  /* Fast 8 bit stereo++ */
  { &RecordingCopy8Sto16S,  AMIGUS_PCM_S_REC_STEREO_8BIT,  AHIST_S16S, 2,  8,  4 },
  /* Fast 16 bit mono */
  { &RecordingCopy16Mto16S, AMIGUS_PCM_S_REC_MONO_16BIT,   AHIST_S16S, 2,  8,  4 },
  /* Fast 16 bit stereo */
  { &RecordingCopy16Sto16S, AMIGUS_PCM_S_REC_STEREO_16BIT, AHIST_S16S, 2,  4,  4 },
  /* Fast 16 bit stereo++ */
  { &RecordingCopy16Sto16S, AMIGUS_PCM_S_REC_STEREO_16BIT, AHIST_S16S, 2,  4,  4 },
  /* HiFi 8 bit mono */
  { &RecordingCopy8Mto16S,  AMIGUS_PCM_S_REC_MONO_8BIT,    AHIST_S16S, 2, 16,  4 },
  /* HiFi 8 bit stereo */
  { &RecordingCopy8Sto16S,  AMIGUS_PCM_S_REC_STEREO_8BIT,  AHIST_S16S, 2,  8,  4 },
  /* HiFi 8 bit stereo++ */
  { &RecordingCopy8Sto16S,  AMIGUS_PCM_S_REC_STEREO_8BIT,  AHIST_S16S, 2,  8,  4 },
  /* HiFi 16 bit mono */
  { &RecordingCopy16Mto16S, AMIGUS_PCM_S_REC_MONO_16BIT,   AHIST_S16S, 2,  8,  4 },
  /* HiFi 16 bit stereo */
  { &RecordingCopy16Sto16S, AMIGUS_PCM_S_REC_STEREO_16BIT, AHIST_S16S, 2,  4,  4 },
  /* HiFi 16 bit stereo++ */
  { &RecordingCopy16Sto16S, AMIGUS_PCM_S_REC_STEREO_16BIT, AHIST_S16S, 2,  4,  4 },
  /* HiFi 24 bit mono */
  { &RecordingCopy24Mto32S, AMIGUS_PCM_S_REC_MONO_24BIT,   AHIST_S32S, 3, 24,  6 },
  /* HiFi 24 bit stereo */
  { &RecordingCopy24Sto32S, AMIGUS_PCM_S_REC_STEREO_24BIT, AHIST_S32S, 3, 24, 12 },
  /* HiFi 24 bit stereo++ */
  { &RecordingCopy24Sto32S, AMIGUS_PCM_S_REC_STEREO_24BIT, AHIST_S32S, 3, 24, 12 }
/*  ^1                      ^2                             ^3          ^4 ^5  ^6
 *
 * Note ^1:
 *   Recording copy function to copy AmiGUS PCM FiFo to AHI buffer.
 * Note ^2:
 *   AmiGUS PCM Recording Sample Format as understood in amigus_hardware.h,
 *   obviously needs to deliver the input of the copy function and provide
 *   the correct AHI sample type as output.
 * Note ^3:
 *   AHI sample type output when recording.
 * Note ^4:
 *   Number of shifts to translate a single AHI Sample to its BYTE size.
 *   Example: Stereo 16bit has 2 x 16 bit = 2 x 2 bytes = 4 bytes = 1<<2.
 * Note ^5:
 *   Number of BYTEs output per single copy function call, depends on the
 *   AmiGUS hardware sample format and mainly of the requirement to read only
 *   LONGs from AmiGUS.The recording buffers' BYTE size needs to be 
 *   multiples of these.
 * Note ^6:
 *   Number of BYTEs ingested per single copy function call, depends on the
 *   CopyFunction - which depends on the AHI and hardware properties.
 */
};
