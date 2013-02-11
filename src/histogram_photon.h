#ifndef HISTOGRAM_PHOTON_
#define HISTOGRAM_PHOTON_

#include <stdio.h>
#include "options.h"
#include "limits.h"
#include "types.h"
#include "correlate_photon.h"

typedef struct {
	size_t n_bins;
	int print_label;
	limits_t limits;
	int scale;
	int (*get_index)(void const *edges, int64_t const value);
	float64_t *bin_edges;
} edges_t;

edges_t *edges_alloc(size_t const n_bins);
int edges_init(edges_t *edges, limits_t const *limits, int const scale,
		int const print_label);
void edges_free(edges_t **edges);

int edges_index_linear(void const *edges, int64_t const value);
int edges_index_log(void const *edges, int64_t const value);
int edges_index_log_zero(void const *edges, int64_t const value);
int edges_index_bsearch(void const *edges, int64_t const value);

typedef struct {
	size_t length;
	int64_t *values;
	combination_t *combination;
} values_vector_t;

values_vector_t *values_vector_alloc(unsigned int const length);
void values_vector_init(values_vector_t *vv);
void values_vector_free(values_vector_t **vv);
int64_t values_vector_index(values_vector_t const *vv, edges_t ** const edges);

typedef struct {
	size_t length;
	int yielded;
	unsigned int *limits;
	unsigned int *values;
} edge_indices_t;

edge_indices_t *edge_indices_alloc(unsigned int const length);
void edge_indices_init(edge_indices_t *edge_indices, edges_t ** const edges);
void edge_indices_free(edge_indices_t **edge_indices);
int edge_indicies_next(edge_indices_t *edge_indices);

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

	uint64_t **counts;

	combinations_t *channels_vectors;
	values_vector_t *values_vector;
	edge_indices_t *edge_indices;

	int (*print)(FILE *stream_out, void const *hist);
	int (*build_channels)(correlation_t const *correlation,
			combination_t *channels);
	int (*build_values)(correlation_t const *correlation, 
			values_vector_t *values);
} histogram_gn_t;

histogram_gn_t *histogram_gn_alloc(options_t const *options);
void histogram_gn_init(histogram_gn_t *hist);
void histogram_gn_free(histogram_gn_t **hist);

int histogram_gn_increment(histogram_gn_t *hist, 
		correlation_t const *correlation);

int histogram_gn_fprintf(FILE *stream_out, void const *hist);

int histogram_photon(FILE *stream_in, FILE *stream_out, 
		options_t const *options);

#endif
