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

void t2v_offset(void *record, offsets_t const *offsets) {
	if ( offsets->offset_time ) {
		((t2_t *)record)->time +=
				offsets->time_offsets[((t2_t *)record)->channel];
	}
}

void t3v_offset(void *record, offsets_t const *offsets) {
	if ( offsets->offset_time ) {
		((t3_t *)record)->time +=
			offsets->time_offsets[((t3_t *)record)->channel];
	}

	if ( offsets->offset_pulse ) {
		((t3_t *)record)->pulse +=
			offsets->pulse_offsets[((t3_t *)record)->channel];
	}
}
