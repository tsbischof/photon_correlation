#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intensity.h"
#include "intensity_photon.h"
#include "modes.h"
#include "error.h"

int intensity_dispatch(FILE *stream_in, FILE *stream_out, options_t *options) {
	return(intensity_photon(stream_in, stream_out, options));
}

counts_t *counts_alloc(int channels) {
	counts_t *counts;
	int result = 0;
	
	counts = (counts_t *)malloc(sizeof(counts_t));
	if ( counts == NULL ) {
		result = -1;
	} else {
		counts->channels = channels;
		counts->window.lower = 0;
		counts->window.upper = 0;
		counts->counts = (uint64_t *)malloc(sizeof(uint64_t)*channels);
		if ( counts->counts == NULL ) {
			counts_free(&counts);
			return(counts);
		}
	}

	return(counts);
}

void counts_init(counts_t *counts) {
	int i;

	for ( i = 0; i < counts->channels; i++ ) {
		counts->counts[i] = 0;
	}
}

void counts_free(counts_t **counts) {
	if ( *counts != NULL ) {
		free((*counts)->counts);
		free(*counts);
	}
}

int counts_increment(counts_t *counts, int channel) {
	if ( channel >= 0 && channel < counts->channels ) {
		counts->counts[channel]++;
		return(PC_SUCCESS);
	} else {
		error("Invalid channel %d requested for increment.\n", channel);
		return(PC_ERROR_CHANNEL);
	}
}

int counts_fscanf(FILE *stream_in, counts_t *counts) {
	int i;
	size_t n_read;

	n_read = fscanf(stream_in, 
				"%"SCNd64",%"SCNd64, 
				&(counts->window.lower),
				&(counts->window.upper));

	if ( n_read != 2) {
		return( feof(stream_in) ? PC_SUCCESS : PC_ERROR_IO );
	}

	for ( i = 0; i < counts->channels; i++ ) {
		n_read = fscanf(stream_in, 
				",%"SCNu64, 
				&(counts->counts[i]));

		if ( n_read != 1 ) {
			return( feof(stream_in) ? PC_SUCCESS : PC_ERROR_IO );
		}
	}

	return(PC_SUCCESS);
}

int counts_fprintf(FILE *stream_out, counts_t const *counts) {
	int i;
	size_t n_write;

	n_write = fprintf(stream_out, 
			"%"PRId64",%"PRId64,
			counts->window.lower,
			counts->window.upper);

	if ( ferror(stream_out) ) {
		return(PC_ERROR_IO);
	}

	for ( i = 0; i < counts->channels; i++ ) {
		n_write = fprintf(stream_out,
				",%"PRIu64,
				counts->counts[i]);
	
		if ( ferror(stream_out) ) {
			return(PC_ERROR_IO);
		}
	}

	fprintf(stream_out, "\n");

	return(PC_SUCCESS);
}

int counts_echo(FILE *stream_in,  FILE *stream_out, int channels) {
	counts_t *counts = counts_alloc(channels);
	counts_next_t next = counts_fscanf;
	counts_print_t print = counts_fprintf;

	if ( counts == NULL ) {
		return(PC_ERROR_MEM);
	} 

	while ( next(stream_in, counts) == PC_SUCCESS ) {
		print(stream_out, counts);
	}

	return(PC_SUCCESS);
}
