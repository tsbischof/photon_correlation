#ifndef CORRELATE_VECTOR_H_
#define CORRELATE_VECTOR_H_

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "histogram_gn.h"
#include "limits.h"
#include "intensity.h"
#include "options.h"

typedef struct {
	int64_t total;
	int64_t number;
} fraction_t;

typedef struct {
	window_t window;
	fraction_t *counts;
} bin_t;

typedef struct {
	int n_bins;
	int channels;
	edges_t *edges;
	bin_t *bin;
} binned_signal_t;

typedef struct {} vector_correlation_t;

typedef struct {
	int channels;
	limits_t limits;
	int n_correlations;
	vector_correlation_t **correlations;
} vector_correlations_t;

int correlate_vector(FILE *stream_in, FILE *stream_out, options_t *options);

vector_correlations_t *allocate_vector_correlations(options_t *options);
void free_vector_correlations(vector_correlations_t **correlations);
void print_vector_correlations(FILE *stream_out, 
		vector_correlations_t *correlations, options_t *options);

binned_signal_t *allocate_binned_signal(options_t *options);
void free_binned_signal(binned_signal_t **signal);
void print_binned_signal(FILE *stream_out, binned_signal_t *signal,
		options_t *options);
int next_binned_signal(FILE *stream_in, binned_signal_t *signal,
		counts_t *counts, options_t *options);
/*void binned_signal_push(binned_signal_t *signal, counts_t *counts);
void binned_signal_pop(binned_signal_t *signal); */
void correlate_binned_signal(vector_correlations_t *correlations,
		binned_signal_t *signal, options_t *options);

#endif
