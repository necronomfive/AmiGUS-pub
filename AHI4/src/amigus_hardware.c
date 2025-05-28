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

#include "amigus_hardware.h"

/******************************************************************************
 * Low-Level hardware access functions - public function definitions.
 *****************************************************************************/

UWORD ReadReg16( APTR card, ULONG offset ) {

  return *(( UWORD * )(( ULONG ) card + offset ));
}

ULONG ReadReg32( APTR card, ULONG offset ) {

  return *(( ULONG * )(( ULONG ) card + offset ));
}

VOID WriteReg16( APTR card, ULONG offset, UWORD value ) {

  *(( UWORD * )(( ULONG ) card + offset )) = value;
}

VOID WriteReg32( APTR card, ULONG offset, ULONG value ) {

  *(( ULONG * )(( ULONG ) card + offset )) = value;
}

/******************************************************************************
 * Low-Level hardware feature lookup tables - public data definitions.
 *****************************************************************************/

const LONG AmiGUSSampleRates[ AMIGUS_PCM_SAMPLE_RATE_COUNT ] = {

   8000, // AMIGUS_PCM_SAMPLE_RATE_8000  @ index 0x0000
  11025, // AMIGUS_PCM_SAMPLE_RATE_11025 @ index 0x0001
  16000, // AMIGUS_PCM_SAMPLE_RATE_16000 @ index 0x0002
  22050, // AMIGUS_PCM_SAMPLE_RATE_22050 @ index 0x0003
  24000, // AMIGUS_PCM_SAMPLE_RATE_24000 @ index 0x0004
  32000, // AMIGUS_PCM_SAMPLE_RATE_32000 @ index 0x0005
  44100, // AMIGUS_PCM_SAMPLE_RATE_44100 @ index 0x0006
  48000, // AMIGUS_PCM_SAMPLE_RATE_48000 @ index 0x0007
  64000  // AMIGUS_PCM_SAMPLE_RATE_64000 @ index 0x0008
  // 96000  // AMIGUS_PCM_SAMPLE_RATE_96000 @ index 0x0009
};

const STRPTR AmiGUSOutputs[ AMIGUS_PCM_OUTPUTS_COUNT ] = {

  "Line Out"
};

const STRPTR AmiGUSInputs[ AMIGUS_PCM_INPUTS_COUNT ] = {

  "External / see Mixer",
  "MHI / Codec",
  "WaveTable",
  // "AHI / PCM",                    // Deactivated - needs splitting 1 card for 2 clients
  "ALL / What-You-Hear"
};

const UWORD AmiGUSInputFlags[ AMIGUS_PCM_INPUTS_COUNT ] = {

  AMIGUS_PCM_S_REC_F_ADC_SRC,        // External above
  AMIGUS_PCM_S_REC_F_CODEC_SRC,
  AMIGUS_PCM_S_REC_F_WAVETABLE_SRC,
  // AMIGUS_PCM_S_REC_F_AHI_SRC,
  AMIGUS_PCM_S_REC_F_MIXER_SRC       // What-You-Hear
};
