/*
 * This file is part of the mhiAmiGUS.library driver.
 *
 * mhiAmiGUS.library driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library driver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "amigus_hardware.h"
#include "debug.h"
#include "SDI_compiler.h"

UWORD ReadReg16( APTR card, ULONG offset ) {

  return *(( UWORD * )(( ULONG ) card + offset ));
}

ULONG ReadReg32( APTR card, ULONG offset ) {

  return *(( ULONG * )(( ULONG ) card + offset ));
}

INLINE UWORD ReadSPI(
  APTR card,
  UWORD SPIregister,
  UWORD blockedSPImask,
  UWORD offsetSPIstatus,
  UWORD offsetSPIaddress,
  UWORD offsetSPIread,
  UWORD offsetSPItrigger ) {

  UWORD status;

  do {

    status = ReadReg16( card, offsetSPIstatus );

  } while ( status & blockedSPImask );
  WriteReg16( card, offsetSPIaddress, SPIregister );
  WriteReg16( card, offsetSPItrigger, AMIGUS_CODEC_SPI_STROBE );
  do {

    status = ReadReg16( card, offsetSPIstatus );

  } while ( status & blockedSPImask );
  return ReadReg16( card, offsetSPIread );
}

UWORD ReadCodecSPI( APTR card, UWORD SPIregister ) {

  return ReadSPI( card,
                  SPIregister,
                  AMIGUS_CODEC_SPI_F_DREQ | AMIGUS_CODEC_SPI_F_BUSY,
                  AMIGUS_CODEC_SPI_STATUS,
                  AMIGUS_CODEC_SPI_ADDRESS, 
                  AMIGUS_CODEC_SPI_READ_DATA,
                  AMIGUS_CODEC_SPI_READ_TRIGGER );
}

VOID WriteReg16( APTR card, ULONG offset, UWORD value ) {

  *(( UWORD * )(( ULONG ) card + offset )) = value;
}

VOID WriteReg32( APTR card, ULONG offset, ULONG value ) {

  *(( ULONG * )(( ULONG ) card + offset )) = value;
}

INLINE VOID WriteSPI(
  APTR card,
  UWORD SPIregister,
  UWORD SPIvalue,
  UWORD blockedSPImask,
  UWORD offsetSPIstatus,
  UWORD offsetSPIaddress,
  UWORD offsetSPIwrite,
  UWORD offsetSPItrigger ) {

  UWORD status;

  do {

    status = ReadReg16( card, offsetSPIstatus );

  } while ( status & blockedSPImask );
  WriteReg16( card, offsetSPIaddress, SPIregister );
  WriteReg16( card, offsetSPIwrite, SPIvalue );
  WriteReg16( card, offsetSPItrigger, AMIGUS_CODEC_SPI_STROBE );
}

VOID WriteCodecSPI( APTR card, UWORD SPIregister, UWORD SPIvalue ) {

  WriteSPI( card,
            SPIregister,
            SPIvalue,
            AMIGUS_CODEC_SPI_F_DREQ | AMIGUS_CODEC_SPI_F_BUSY,
            AMIGUS_CODEC_SPI_STATUS,
            AMIGUS_CODEC_SPI_ADDRESS, 
            AMIGUS_CODEC_SPI_WRITE_DATA,
            AMIGUS_CODEC_SPI_WRITE_TRIGGER );
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

  "External / see Mixer",
  "MHI / Codec",
  "WaveTable",
  // "AHI / PCM",                    // Requires splitting 1 card for 2 clients
  "ALL / What-You-Hear"
};

const UWORD AmiGUSInputFlags[ AMIGUS_INPUTS_COUNT ] = {

  AMIGUS_PCM_S_REC_F_ADC_SRC,        // External above
  AMIGUS_PCM_S_REC_F_CODEC_SRC,
  AMIGUS_PCM_S_REC_F_WAVETABLE_SRC,
  // AMIGUS_PCM_S_REC_F_AHI_SRC,
  AMIGUS_PCM_S_REC_F_MIXER_SRC       // What-You-Hear
};

/*
 * Some equalizer support info...
 * AmigaAMP sliders |  MHI EQ5   |         AmiGUS        | Industry
 *        # |   Hz  | ID |   Hz  | calculated | possible | Standard
 * ---------+-------+----+-------+------------+----------+----------
 *        1 |    60 |  3 |    64 |            |          |
 *        2 |   170 |  3 |    64 |        230 |      150 |      125
 *        3 |   310 |  7 |   250 |            |          |
 *        4 |   600 |  7 |   250 |        775 |      775 |      500
 *        5 |  1000 |  4 |  1000 |            |          |
 *        6 |  3000 |  4 |  1000 |       4243 |     4243 |     2000
 *        7 |  6000 |  8 |  4000 |            |          |
 *        8 | 12000 |  8 |  4000 |      12961 |    12961 |     8000
 *        9 | 14000 |  5 | 16000 |            |          |
 *       10 | 16000 |  5 | 16000 |            |          |
 *
 * The borders between the frequency bands would be calculated using
 * log-average: 10^( ( log(a)/log(10) + log(b)/log(10) ) / 2 )
 *
 * So much for defining other frequency patterns like:
 */
const WORD AmiGUSDefaultEqualizer[ 9 ] = {
  0, /* +/- 0dB */   125, /* Hz */
  0, /* +/- 0dB */   500, /* Hz */
  0, /* +/- 0dB */  2000, /* Hz */
  0, /* +/- 0dB */  8000, /* Hz */
  0  /* +/- 0dB */
};

const WORD AmiGUSAmigaAmpEqualizer[ 9 ] = {
  0, /* +/- 0dB */   150, /* Hz */
  0, /* +/- 0dB */   775, /* Hz */
  0, /* +/- 0dB */  4243, /* Hz */
  0, /* +/- 0dB */ 12961, /* Hz */
  0  /* +/- 0dB */
};

const UBYTE AmiGUSVolumeMapping[ 104 ] = {
  0xFE /*  0% */, 
  72 /*  1% */, 66 /*  2% */, 60 /*  3% */, 54 /*  4% */, 51 /*   5% */,
  48 /*  6% */, 45 /*  7% */, 42 /*  8% */, 41 /*  9% */, 39 /*  10% */,
  37 /* 11% */, 36 /* 12% */, 34 /* 13% */, 33 /* 14% */, 32 /*  15% */,
  31 /* 16% */, 30 /* 17% */, 30 /* 18% */, 29 /* 19% */, 28 /*  20% */,
  27 /* 21% */, 26 /* 22% */, 26 /* 23% */, 25 /* 24% */, 24 /*  25% */,
  23 /* 26% */, 23 /* 27% */, 22 /* 28% */, 21 /* 29% */, 21 /*  30% */,
  20 /* 31% */, 20 /* 32% */, 19 /* 33% */, 19 /* 34% */, 18 /*  35% */,
  18 /* 36% */, 17 /* 37% */, 17 /* 38% */, 16 /* 39% */, 16 /*  40% */,
  15 /* 41% */, 15 /* 42% */, 15 /* 43% */, 14 /* 44% */, 14 /*  45% */,
  13 /* 46% */, 13 /* 47% */, 12 /* 48% */, 12 /* 49% */, 12 /*  50% */,
  11 /* 51% */, 11 /* 52% */, 11 /* 53% */, 10 /* 54% */, 10 /*  55% */,
  10 /* 56% */,  9 /* 57% */,  9 /* 58% */,  8 /* 59% */,  8 /*  60% */,
   8 /* 61% */,  7 /* 62% */,  7 /* 63% */,  7 /* 64% */,  7 /*  65% */,
   6 /* 66% */,  6 /* 67% */,  6 /* 68% */,  6 /* 69% */,  6 /*  70% */,
   5 /* 71% */,  5 /* 72% */,  5 /* 73% */,  5 /* 74% */,  5 /*  75% */,
   4 /* 76% */,  4 /* 77% */,  4 /* 78% */,  4 /* 79% */,  4 /*  80% */,
   3 /* 81% */,  3 /* 82% */,  3 /* 83% */,  3 /* 84% */,  3 /*  85% */,
   2 /* 86% */,  2 /* 87% */,  2 /* 88% */,  2 /* 89% */,  2 /*  90% */,
   1 /* 91% */,  1 /* 92% */,  1 /* 93% */,  1 /* 94% */,  1 /*  95% */,
   0 /* 96% */,  0 /* 97% */,  0 /* 98% */,  0 /* 99% */,  0 /* 100% */,
  99          , 99          , 99 // padding back to LONGs
};