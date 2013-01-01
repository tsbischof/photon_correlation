#include <math.h>

#include "correlate_vector.h"
#include "error.h"

/* Tools to read in an correlate a vector of values, rather than photons. This 
 * probably exists in other libraries, but it would be worthwhile to have a
 * simple implementation here.
 */

int correlate_vector(FILE *stream_in, FILE *stream_out, options_t *options) {
	int result = 0;
	counts_t *counts;
	vector_correlations_t *correlations;
	binned_signal_t *signal;

	counts = allocate_counts(options->channels);
	correlations = allocate_vector_correlations(options);
	signal = allocate_binned_signal(options);

	if ( counts == NULL || correlations == NULL || signal == NULL ) {
		result = -1;
	} else {
		while ( ! next_binned_signal(stream_in, signal, counts, options) ) {
			print_binned_signal(stream_out, signal, options);
//			correlate_binned_signal(correlations, signal, options);
		}
	}

	if ( ! result ) {
		print_vector_correlations(stream_out, correlations, options);
	}

	debug("Freeing counts.\n");
	free_counts(&counts);
	debug("Freeing vector correlations.\n");
	free_vector_correlations(&correlations);
	debug("Freeing binned signal.\n");
	free_binned_signal(&signal);
	return(result);
}

/* Perform the implementation by allocating a fixed queue of entries, as needed
 * to perform all levels of correlations. Have a bunch of pointers to these 
 * entries, such that, when two valid values are found, the correlations
 * at that delay can be incremented. Go through this until the stream runs
 * dry. 
 */

vector_correlations_t *allocate_vector_correlations(options_t *options) {
	vector_correlations_t *correlations = NULL;

	return(correlations);
}

void free_vector_correlations(vector_correlations_t **correlations) {
}

void print_vector_correlations(FILE *stream_out, 
		vector_correlations_t *correlations, options_t *options) {
}

int next_binned_signal(FILE *stream_in, binned_signal_t *signal,
		counts_t *counts, options_t *options) {
	int result = 0;

	if ( ! (result = next_counts(stream_in, counts, options)) ) {
//		binned_signal_push(signal, counts);
	}
	return(-1);
}

void correlate_binned_signal(vector_correlations_t *correlations,
		binned_signal_t *signal, options_t *options) {
}

binned_signal_t *allocate_binned_signal(options_t *options) { 
	binned_signal_t *signal = NULL;
	int result;
	int i;
	int j;

	signal = (binned_signal_t *)malloc(sizeof(binned_signal_t));

	if ( signal == NULL ) {
		result = -1;
	} else {
		signal->n_bins = options->time_limits.bins;
		signal->channels = options->channels;
		signal->edges = allocate_edges(options->time_limits.bins);
		signal->bin = (bin_t *)malloc(sizeof(bin_t)*signal->n_bins);

		if ( signal->bin == NULL ) {
			result = -1;
		} else {
			result = edges_from_limits(signal->edges, &(options->time_limits),
					options->time_scale);
		}

		for ( i = 0; ! result && i < signal->n_bins; i++ ) {
			signal->bin[i].window.lower = (int64_t)floor(
					signal->edges->bin_edges[i]);
			signal->bin[i].window.upper = (int64_t)floor(
					signal->edges->bin_edges[i+1]);

			signal->bin[i].counts = (fraction_t *)malloc(
					sizeof(fraction_t)*signal->channels);
			if ( signal->bin[i].counts == NULL ) {
				result = -1;
			} else {
				for ( j = 0; j < signal->channels; j++ ) {
					signal->bin[i].counts[j].total = 0;
					signal->bin[i].counts[j].number = 0;
				}
			}
		}
	}

	if ( result ) {
		free_binned_signal(&signal);
		signal = NULL;
	}

	return(signal);
}

void free_binned_signal(binned_signal_t **signal) {
	int i;

	if ( *signal != NULL ) {
		free_edges(&((*signal)->edges));
		if ( (*signal)->bin != NULL ) {
			for ( i = 0; i < (*signal)->n_bins; i++ ) {
				free((*signal)->bin[i].counts);
			}
			free((*signal)->bin);
		}
		free(*signal);
	}
}

void print_binned_signal(FILE *stream_out, binned_signal_t *signal, 
		options_t *options) {
	int i, j;

	if ( options->binary_out ) {
		error("Binary output not yet supported.\n");
	} else {
		for ( i = 0; i < signal->n_bins; i++ ) {
			fprintf(stream_out, "%"PRIf64",%"PRIf64,
					signal->edges->bin_edges[i],
					signal->edges->bin_edges[i+1]);
			for ( j = 0; j < signal->channels; j++ ) {
				fprintf(stream_out, ",%"PRId64"/%"PRId64,
						signal->bin[i].counts[j].total,
						signal->bin[i].counts[j].number);
			}
			fprintf(stream_out, "\n");
		}
	}
}
