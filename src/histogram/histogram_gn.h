#ifndef HISTOGRAM_GN_
#define HISTOGRAM_GN_

#include <stdio.h>
#include "../options.h"
#include "edges.h"
#include "values_vector.h"
#include "../correlation/correlation.h"
#include "../combinatorics/combinations.h"

typedef struct {
	int channels;
	int order;
	int mode;
	unsigned int dimensions;
	size_t n_bins;
	int n_histograms;

	int time_scale;
	int pulse_scale;

	edges_t **edges;

	unsigned long long **counts;

	combination_t *channels_vector;
	values_vector_t *values_vector;
	edge_indices_t *edge_indices;

	int (*print)(FILE *stream_out, void const *hist);
	int (*build_channels)(correlation_t const *correlation,
			combination_t *channels);
	int (*build_values)(correlation_t const *correlation, 
			values_vector_t *values);
} histogram_gn_t;

histogram_gn_t *histogram_gn_alloc(int const mode, unsigned int const order,
		unsigned int const channels,
		int const time_scale, limits_t const *time_limits,
		int const pulse_scale, limits_t const *pulse_limits);
void histogram_gn_init(histogram_gn_t *hist);
void histogram_gn_free(histogram_gn_t **hist);

int histogram_gn_increment(histogram_gn_t *hist, 
		correlation_t const *correlation);

int histogram_gn_fprintf(FILE *stream_out, void const *hist);

#endif
