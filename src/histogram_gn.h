#ifndef HISTOGRAM_GN_
#define HISTOGRAM_GN_

#include <stdio.h>
#include "options.h"
#include "limits.h"
#include "types.h"

typedef struct {
	int n_bins;
	int print_label;
	char dimension_label[20];
	limits_t limits;
	int scale;
	float64_t *bin_edges;
} edges_t;

typedef struct {
	int n_dimensions;
	char histogram_label[20];
	int n_bins;
	int *index_bases;
	edges_t **dimensions;
	unsigned int *counts;
} gn_histogram_t;

edges_t *allocate_edges(int n_bins);
void free_edges(edges_t **edges);
int edges_get_index(edges_t *edges, int64_t value);
int edge_index_linear(edges_t *edges, int64_t value);
int edge_index_log(edges_t *edges, int64_t value);
int edge_index_bsearch(edges_t *edges, int64_t value);
int edges_from_limits(edges_t *edges, limits_t *limits, int scale);
void print_edges(FILE *stream_out, edges_t *edges);

gn_histogram_t *allocate_gn_histogram(int n_dimensions, edges_t **dimensions);
void free_gn_histogram(gn_histogram_t **histogram);

int gn_histogram_make_n_bins(gn_histogram_t *histogram);
int gn_histogram_make_index_bases(gn_histogram_t *histogram);
int gn_histogram_get_index(gn_histogram_t *histogram, 
		int64_t *values);
int gn_histogram_get_index_from_indices(gn_histogram_t *histogram,
		int *indices);
int gn_histogram_increment(gn_histogram_t *histogram,
		int64_t *values);
void print_gn_histogram(FILE *stream_out, gn_histogram_t *histogram, 
		options_t *options);
int gn_histogram_next_index(gn_histogram_t *histogram, int *indices);

#endif
