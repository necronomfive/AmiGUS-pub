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

#include "amigus_hardware.h"

UWORD ReadReg16( APTR amiGUS, ULONG offset ) {

  return *(( UWORD * )(( ULONG ) amiGUS + offset ));
}

void WriteReg16( APTR amiGUS, ULONG offset, UWORD value ) {

  *(( UWORD * )(( ULONG ) amiGUS + offset )) = value;
}

ULONG ReadReg32( APTR amiGUS, ULONG offset ) {

  return *(( ULONG * )(( ULONG ) amiGUS + offset ));
}

void WriteReg32( APTR amiGUS, ULONG offset, ULONG value ) {

  *(( ULONG * )(( ULONG ) amiGUS + offset )) = value;
}

/*
 * ---------------------------------------------------
 * Data definitions
 * ---------------------------------------------------
 */

/*
 * Array of supported sample rates, shall always be in 
 * sync with 
 * - AMIGUS_SAMPLE_RATE_* in amigus_hardware.h and
 * - AMIGUS_AHI_NUM_SAMPLE_RATES in amigus_public.h.
 */
const LONG AmiGUSSampleRates[ AMIGUS_PCM_SAMPLE_RATE_COUNT ] = {

   8000, // AMIGUS_PCM_SAMPLE_RATE_8000  @ index 0x0000
  11025, // AMIGUS_PCM_SAMPLE_RATE_11025 @ index 0x0001
  16000, // AMIGUS_PCM_SAMPLE_RATE_16000 @ index 0x0002
  22050, // AMIGUS_PCM_SAMPLE_RATE_22050 @ index 0x0003
  24000, // AMIGUS_PCM_SAMPLE_RATE_24000 @ index 0x0004
  32000, // AMIGUS_PCM_SAMPLE_RATE_32000 @ index 0x0005
  44100, // AMIGUS_PCM_SAMPLE_RATE_44100 @ index 0x0006
  48000, // AMIGUS_PCM_SAMPLE_RATE_48000 @ index 0x0007
  96000  // AMIGUS_PCM_SAMPLE_RATE_96000 @ index 0x0008
};

const STRPTR AmiGUSOutputs[ AMIGUS_OUTPUTS_COUNT ] = {

  "Line Out"
};

const STRPTR AmiGUSInputs[ AMIGUS_INPUTS_COUNT ] = {

  "AHI / PCM",
  "MHI / Codec",
  "WaveTable",
  "External / see Mixer",
  "ALL / What-You-Hear"
};

/*
 * Array of sample formats per hardware sample format ID - for playback.
 */
const WORD AmiGUSPlaybackSampleFormat[ AMIGUS_PCM_S_PLAY_FORMAT_COUNT ] = {

  AMIGUS_PCM_S_PLAY_MONO_8BIT,      // @ index 0 = value 0x0000
  AMIGUS_PCM_S_PLAY_STEREO_8BIT,    // @ index 1 = value 0x0001
  AMIGUS_PCM_S_PLAY_MONO_16BIT,     // @ index 2 = value 0x0002
  AMIGUS_PCM_S_PLAY_STEREO_16BIT,   // @ index 3 = value 0x0003
  AMIGUS_PCM_S_PLAY_MONO_24BIT,     // @ index 4 = value 0x0004
  AMIGUS_PCM_S_PLAY_STEREO_24BIT    // @ index 5 = value 0x0005
};

/*
 * Array of sample sizes in BYTE per hardware sample format ID - for playback.
 */
const UWORD AmiGUSPlaybackSampleSizes[ AMIGUS_PCM_S_PLAY_FORMAT_COUNT ] = {

  1, // AMIGUS_PCM_S_PLAY_MONO_8BIT    @ index 0 = value 0x0000
  2, // AMIGUS_PCM_S_PLAY_STEREO_8BIT  @ index 1 = value 0x0001
  2, // AMIGUS_PCM_S_PLAY_MONO_16BIT   @ index 2 = value 0x0002
  4, // AMIGUS_PCM_S_PLAY_STEREO_16BIT @ index 3 = value 0x0003
  3, // AMIGUS_PCM_S_PLAY_MONO_24BIT   @ index 4 = value 0x0004
  6  // AMIGUS_PCM_S_PLAY_STEREO_24BIT @ index 5 = value 0x0005
};

/*
 * Array of sample formats per hardware sample format ID - for recording.
 */
const WORD AmiGUSRecordingSampleFormat[ AMIGUS_PCM_S_REC_FORMAT_COUNT ] = {

  AMIGUS_PCM_S_REC_MONO_8BIT,       // @ index 0 = value 0x0006
  AMIGUS_PCM_S_REC_STEREO_8BIT,     // @ index 1 = value 0x0000
  AMIGUS_PCM_S_REC_MONO_16BIT,      // @ index 2 = value 0x0007
  AMIGUS_PCM_S_REC_STEREO_16BIT     // @ index 3 = value 0x0001
};

/*
 * Array of sample sizes in BYTE per hardware sample format ID - for recording.
 */
const UWORD AmiGUSRecordingSampleSizes[ AMIGUS_PCM_S_REC_FORMAT_COUNT ] = {

  1, // AMIGUS_PCM_S_REC_MONO_8BIT     @ index 0 = value 0x0006
  2, // AMIGUS_PCM_S_REC_STEREO_8BIT   @ index 1 = value 0x0000
  2, // AMIGUS_PCM_S_REC_MONO_16BIT    @ index 2 = value 0x0007
  4  // AMIGUS_PCM_S_REC_STEREO_16BIT  @ index 3 = value 0x0001
};