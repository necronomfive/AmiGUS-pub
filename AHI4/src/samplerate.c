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
#include <limits.h>

#include "amigus_hardware.h"
#include "debug.h"
#include "samplerate.h"

LONG FindSampleRateIdForValue( LONG sampleRate ) {

  LONG prev;
  LONG next = LONG_MAX;
  LONG i = 0;

  while ( AMIGUS_PCM_SAMPLE_RATE_COUNT > i ) {

    prev = next;
    next = AmiGUSSampleRates[ i ] - sampleRate;

    /* We need absolute difference only. */
    if (0 > next) {
      next = -next;
    }

    LOG_D(("D: Frequency diff was %ld, next %ld\n", prev, next));
    if ( prev < next ) {
      /* Since frequencies are ordered, stop when difference is increasing. */
      break;
    }
    ++i;
  }
  LOG_I(("I: Using %ldHz aka ID %ld = 0x%02lx for requested %ldHz\n", 
         AmiGUSSampleRates[ i - 1 ],
         i - 1,
         i - 1,
         sampleRate));
  return i - 1;
}

LONG FindSampleRateValueForId( LONG id ) {

  LONG result = -1;
  if (( 0 <= id) &&
      ( AMIGUS_PCM_SAMPLE_RATE_COUNT > id)) {

    result = AmiGUSSampleRates[ id ];
  }
  LOG_D(("D: Using %ldHz for ID %ld = 0x%02lx\n", result, id, id));
  return result;
}
