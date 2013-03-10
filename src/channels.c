#include "channels.h"

#include "options.h"
#include "error.h"
#include "channels_photon.h"
#include "t2.h"
#include "t3.h"

int channels_dispatch(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	return(channels_photon(stream_in, stream_out, options));
}

int64_t offset_difference(int64_t const *offsets, int const channels) {
	int64_t min = 0;
	int64_t max = 0;
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

offsets_t *offsets_alloc(int const channels) {
	offsets_t *offsets = NULL;

	offsets = (offsets_t *)malloc(sizeof(offsets_t));

	if ( offsets == NULL ) {
		return(offsets);
	}

	offsets->channels = channels;
	offsets->time_offsets = (int64_t *)malloc(sizeof(int64_t)*channels);
	offsets->pulse_offsets = (int64_t *)malloc(sizeof(int64_t)*channels);

	if ( offsets->time_offsets == NULL || offsets->pulse_offsets == NULL ) {
		offsets_free(&offsets);
		return(offsets);
	} else {
		return(offsets);
	}
}

void offsets_init(offsets_t *offsets, pc_options_t const *options) {
	int i;

	offsets->offset_time = options->offset_time;
	offsets->offset_pulse = options->offset_pulse;

	for ( i = 0; offsets->offset_time && i < offsets->channels; i++ ) {
		offsets->time_offsets[i] = options->time_offsets[i];
	} 

	for ( i = 0; offsets->offset_pulse && i < offsets->channels; i++ ) {
		offsets->pulse_offsets[i] = options->pulse_offsets[i];
	}
}

void offsets_free(offsets_t **offsets) {
	if ( *offsets != NULL ) {
		free((*offsets)->time_offsets);
		free((*offsets)->pulse_offsets);
		free(*offsets);
	}
}
