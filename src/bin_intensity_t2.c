#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#include "files.h"
#include "modes.h"
#include "error.h"
#include "strings.h"
#include "t2.h"
#include "t3.h"
#include "histogram_gn.h"
#include "bin_intensity.h"
#include "bin_intensity_t2.h"

int bin_intensity_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	t2_t *queue;
	counts_t2_t *counts;
	int result = 0;
	int done = 0;

	queue = (t2_t *)malloc(options->queue_size*sizeof(t2_t));
	counts = allocate_counts_t2(options->channels, &(options->time_limits));

	if ( counts == NULL || queue == NULL ) {
		result = -1;
	}

	while ( ! done && next_counts_t2_queue(in_stream, queue, options) ) {
		done = count_t2(queue, options);
	}

	print_counts_t2(out_stream, counts);

	debug("Cleaning up.\n");
	free_counts_t2(&counts);
	free(queue);
	return(result);
}

counts_t2_t *allocate_counts_t2(int channels, limits_t *time_limits) {
/*	counts_t *counts;
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

	return(counts); */
	return(0);
}

void init_counts_t2(counts_t2_t *counts) {
/*
	int i;

	for ( i = 0; i < counts->channels; i++ ) {
		counts->counts[i] = 0;
	}
*/
}

void free_counts_t2(counts_t2_t **counts) {
/*	if ( *counts != NULL ) {
		free((*counts)->counts);
		free(*counts);
	} */
}

int increment_counts_t2(counts_t2_t *counts, int channel) {
/*	if ( channel >= 0 && channel < counts->channels ) {
		counts->counts[channel] += 1;
		return(0);
	} else {
		error("Invalid channel %d requested for increment.\n", channel);
		return(-1);
	}*/
	return(0);
}

int next_counts_t2_queue(FILE *in_stream, t2_t *queue, options_t *options) {
	return(-1);
}

void print_counts_t2(FILE *out_stream, counts_t2_t *counts) {
}

int count_t2(t2_t *queue, options_t *options) {
	return(-1);
}
