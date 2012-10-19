#include "correlate_vector.h"
#include "intensity.h"
#include "error.h"

/* Tools to read in an correlate a vector of values, rather than photons. This 
 * probably exists in other libraries, but it would be worthwhile to have a
 * simple implementation here.
 */

int correlate_vector(FILE *in_stream, FILE *out_stream, options_t *options) {
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
		while ( ! next_binned_signal(in_stream, signal, counts, options) ) {
			correlate_binned_signal(correlations, signal, options);
		}
	}

	if ( ! result ) {
		print_vector_correlations(out_stream, correlations, options);
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

void print_vector_correlations(FILE *out_stream, 
		vector_correlations_t *correlations, options_t *options) {
}

int next_binned_signal(FILE *in_stream, binned_signal_t *signal,
		counts_t *counts, options_t *options) {
	return(-1);
}

void correlate_binned_signal(vector_correlations_t *correlations,
		binned_signal_t *signal, options_t *options) {
}

binned_signal_t *allocate_binned_signal(options_t *options) { 
	binned_signal_t *signal = NULL;

	return(signal);
}

void free_binned_signal(binned_signal_t **signal) {
}
