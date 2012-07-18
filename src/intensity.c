#include <stdlib.h>
#include <string.h>

#include "intensity.h"
#include "intensity_t2.h"
#include "intensity_t3.h"
#include "modes.h"
#include "error.h"

int intensity_dispatch(FILE *in_stream, FILE *out_stream, options_t *options) {
	int result;

	debug("Checking the mode.\n");
	if ( options->mode == MODE_T2 ) {
		debug("Mode t2.\n");
		result = intensity_t2(in_stream, out_stream, options);
	} else if ( options->mode == MODE_T3 ) {
		debug("Mode t3.\n");
		result = intensity_t3(in_stream, out_stream, options);
	} else {
		result = -1;
	}

	return(result);
}

counts_t *allocate_counts(int channels) {
	counts_t *counts;
	int result = 0;
	
	counts = (counts_t *)malloc(sizeof(counts_t));
	if ( counts == NULL ) {
		result = -1;
	} else {
		counts->channels = channels;
		counts->counts = (long long int *)malloc(
				sizeof(long long int)*channels);
		if ( counts->counts == NULL ) {
			result = -1;
		}
	}

	if ( result ) {
		free_counts(&counts);
		counts = NULL;
	}

	return(counts);
}

void init_counts(counts_t *counts) {
	int i;

	for ( i = 0; i < counts->channels; i++ ) {
		counts->counts[i] = 0;
	}
}

void free_counts(counts_t **counts) {
	if ( *counts != NULL ) {
		free((*counts)->counts);
		free(*counts);
	}
}

int increment_counts(counts_t *counts, int channel) {
	if ( channel >= 0 && channel < counts->channels ) {
		counts->counts[channel] += 1;
		return(0);
	} else {
		error("Invalid channel %d requested for increment.\n", channel);
		return(-1);
	}
}

void print_counts(FILE *out_stream, long long int lower_time,
		long long int upper_time,  counts_t *counts) {
	int i;

	fprintf(out_stream, "%lld,%lld,", lower_time, upper_time);
	for ( i = 0; i < counts->channels; i++ ) {
		fprintf(out_stream, "%lld", counts->counts[i]);
		if ( i != counts->channels - 1 ) {
			fprintf(out_stream, ",");
		}
	}
	fprintf(out_stream, "\n");
}
