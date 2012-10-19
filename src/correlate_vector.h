#ifndef CORRELATE_VECTOR_H_
#define CORRELATE_VECTOR_H_

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "intensity.h"
#include "options.h"

typedef struct {
	int64_t total;
	int64_t number;
} fraction_t;

typedef struct {
	int n_bins;
	int *queue_limits;
	fraction_t *intensity;
} binned_signal_t;

typedef struct {} vector_correlation_t;

typedef struct {
	int order;
	int channels;
	limits_t limits;
	int n_correlations;
	vector_correlation_t *correlations;
} vector_correlations_t;

int correlate_vector(FILE *in_stream, FILE *out_stream, options_t *options);

vector_correlations_t *allocate_vector_correlations(options_t *options);
void free_vector_correlations(vector_correlations_t **correlations);
void print_vector_correlations(FILE *out_stream, 
		vector_correlations_t *correlations, options_t *options);

int next_binned_signal(FILE *in_stream, binned_signal_t *signal,
		counts_t *counts, options_t *options);
void correlate_binned_signal(vector_correlations_t *correlations,
		binned_signal_t *signal, options_t *options);

binned_signal_t *allocate_binned_signal(options_t *options);
void free_binned_signal(binned_signal_t **signal);

#endif
