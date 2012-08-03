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

/* The basic procedure here is as follows:
 * 1. Determine the maximum time distance between photons as the difference
 *    between the lowest lower limit for a bin and the highest upper limit.
 * 2. Collect photons until this limit is exceeded or the stream ends.
 * 3. For the first photon in the stream, determine whether its time is 
 *    satisfactory (time + lower limit >= start time, 
 *    time + upper limit < end time).
 */
int bin_intensity_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	t2_queue_t *queue;
	t2_counts_t *counts;
	int result = 0;
	int done = 0;

	queue = allocate_t2_queue(options->queue_size);
	counts = allocate_t2_counts(options->channels, options);

	if ( counts == NULL || queue == NULL ) {
		result = -1;
	}

	while ( ! done && 
				next_t2_counts_queue(in_stream, queue, counts, options) ) {
		done = count_t2(queue, options);
	}

	print_t2_counts(out_stream, counts);

	debug("Cleaning up.\n");
	free_t2_counts(&counts);
	free_t2_queue(&queue);
	return(result);
}

t2_counts_t *allocate_t2_counts(int channels, options_t *options) {
	t2_counts_t *counts = NULL;
	int result = 0;
	int i;

	counts = (t2_counts_t *)malloc(sizeof(t2_counts_t));

	if ( counts == NULL ) {
		result = -1;
	} else {
		counts->channels = options->channels;
		counts->bins = options->time_limits.bins;
		counts->bin_edges = allocate_edges(options->time_limits.bins);
		counts->bin_counts = (bin_counts_t *)malloc(sizeof(bin_counts_t)
				*options->time_limits.bins);

		if ( counts->bin_edges == NULL || counts->bin_counts == NULL ) {
			result = -1;
		}

		for ( i = 0; !result && i < options->time_limits.bins; i++ ) {
			counts->bin_counts[i].counts = (long long int *)malloc(
					sizeof(long long int)*options->channels);

			if ( counts->bin_counts[i].counts == NULL ) {
				result = -1;
			}
		}
	}

	if ( result ) {
		free_t2_counts(&counts);
		counts = NULL;
	}
		
	return(counts);
}

void init_t2_counts(t2_counts_t *counts, options_t *options) {
	int i,j;

	edges_from_limits(counts->bin_edges, &(options->time_limits), 
			options->time_scale);
	
	for ( i = 0; i < counts->bins; i++ ) {
		counts->bin_counts[i].limits.lower = 0;
		counts->bin_counts[i].limits.bins = 0;
		counts->bin_counts[i].limits.upper = 0;

		for ( j = 0; j < counts->channels; j++ ) {
			counts->bin_counts[i].counts[j] = 0;	
		}
	}
}

void free_t2_counts(t2_counts_t **counts) {
	int i;
	if ( *counts != NULL ) {
		if ( (*counts)->bin_counts != NULL ) {
			for ( i = 0; i < (*counts)->bins; i++ ) {
				free((*counts)->bin_counts[i].counts);
			}
		}
		free(&(*counts)->bin_counts);

		free_edges(&(*counts)->bin_edges);
	} 

	free(*counts);
}

int next_t2_counts_queue(FILE *in_stream, t2_queue_t *queue, 
			t2_counts_t *counts, options_t *options) {
	return(-1);
}

void print_t2_counts(FILE *out_stream, t2_counts_t *counts) {
}

int count_t2(t2_queue_t *queue, options_t *options) {
	return(-1);
}
