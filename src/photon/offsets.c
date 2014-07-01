/*
 * Copyright (c) 2011-2014, Thomas Bischof
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Massachusetts Institute of Technology nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>

#include "offsets.h"
#include "../error.h"

#include "t2.h"
#include "t3.h"

offsets_t *offsets_alloc(unsigned int const channels) {
	offsets_t *offsets = NULL;

	offsets = (offsets_t *)malloc(sizeof(offsets_t));

	if ( offsets == NULL ) {
		return(offsets);
	}

	offsets->channels = channels;
	offsets->time_offsets = (long long *)malloc(sizeof(long long)*channels);
	offsets->pulse_offsets = (long long *)malloc(sizeof(long long)*channels);

	if ( offsets->time_offsets == NULL || offsets->pulse_offsets == NULL ) {
		offsets_free(&offsets);
		return(offsets);
	} 

	return(offsets);
}

void offsets_init(offsets_t *offsets, 
		int const offset_time, long long const *time_offsets,
		int const offset_pulse, long long const *pulse_offsets) {
	int i;

	offsets->offset_time = offset_time;
	offsets->offset_pulse = offset_pulse;

	for ( i = 0; offset_time && i < offsets->channels; i++ ) {
		offsets->time_offsets[i] = time_offsets[i];
	} 

	for ( i = 0; offset_pulse && i < offsets->channels; i++ ) {
		offsets->pulse_offsets[i] = pulse_offsets[i];
	}
}

void offsets_free(offsets_t **offsets) {
	if ( *offsets != NULL ) {
		free((*offsets)->time_offsets);
		free((*offsets)->pulse_offsets);
		free(*offsets);
	}
}

long long offset_span(long long const *offsets, unsigned int const channels) {
	long long min = 0;
	long long max = 0;
	int i;

	for ( i = 0; i < channels; i++ ) {
		if ( offsets[i] < min ) {
			min = offsets[i];
		} 
		if ( offsets[i] > max ) {
			max = offsets[i];
		}
	}

	return(max-min);
}

void t2_offset(photon_t *record, offsets_t const *offsets) {
	if ( offsets->offset_time ) {
		record->t2.time +=
				offsets->time_offsets[record->t2.channel];
	}
}

void t3_offset(photon_t *record, offsets_t const *offsets) {
	if ( offsets->offset_time ) {
		record->t3.time +=
			offsets->time_offsets[record->t3.channel];
	}

	if ( offsets->offset_pulse ) {
		record->t3.pulse +=
			offsets->pulse_offsets[record->t3.channel];
	}
}
