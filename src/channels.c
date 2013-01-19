#include "channels.h"

#include "options.h"
#include "error.h"
#include "modes.h"
#include "t2.h"
#include "t3.h"
#include "channels_t2.h"
#include "channels_t3.h"
#include "channels_photon.h"

int channels_dispatch(FILE *stream_in, FILE *stream_out, options_t *options) {
	if ( options->use_void ) {
		return(channels_photon(stream_in, stream_out, options));
	} else if ( options->mode == MODE_T2 ) {
		return(channels_t2(stream_in, stream_out, options));
	} else if ( options->mode == MODE_T3 ) {
		return(channels_t3(stream_in, stream_out, options));
	} else {
		error("Mode not supported: %s\n", options->mode_string);
		return(PC_ERROR_MODE);
	}
}

int64_t offset_difference(int64_t *offsets, int n) {
	int64_t min = 0;
	int64_t max = 0;
	int i;

	for ( i = 0; i < n; i++ ) {
		if ( offsets[i] < min ) {
			min = offsets[i];
		} 
		if ( offsets[i] > max ) {
			max = offsets[i];
		}
	}

	return(max-min);
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

void t2v_offset(void *record, offsets_t const *offsets) {
	if ( offsets->offset_time ) {
		((t2_t *)record)->time +=
				offsets->time_offsets[((t2_t *)record)->channel];
	} 
}

offsets_t *offsets_alloc(int channels) {
	offsets_t *offsets = NULL;

	offsets->time_offsets = (int64_t *)malloc(sizeof(int64_t)*channels);
	offsets->pulse_offsets = (int64_t *)malloc(sizeof(int64_t)*channels);

	if ( offsets->time_offsets == NULL || offsets->pulse_offsets == NULL ) {
		offsets_free(&offsets);
		return(offsets);
	} else {
		return(offsets);
	}
}

void offsets_init(offsets_t *offsets, options_t *options) {
	int i;

	offsets->offset_time = options->offset_time;
	options->offset_pulse = options->offset_pulse;

	for ( i = 0; i < options->channels; i++ ) {
		offsets->time_offsets[i] = options->time_offsets[i];
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
