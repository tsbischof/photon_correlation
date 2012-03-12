#ifndef HISTOGRAM_GN_
#define HISTOGRAM_GN_

#include <stdio.h>

#define SCALE_UNKNOWN -1
#define SCALE_LINEAR 1
#define SCALE_LOG 2
#define SCALE_LOG_ZERO 3

typedef struct {
	long long int lower;
	int bins;
	long long int upper;
} limits_t;

typedef struct {
	int n_bins;
	int print_label;
	char dimension_label[20];
	double *bin_edges;
} edges_t;

typedef struct {
	int n_dimensions;
	char histogram_label[20];
	int n_bins;
	int *index_bases;
	edges_t **dimensions;
	unsigned int *counts;
} gn_histogram_t;

int str_to_limits(char *str, limits_t *limits);
int scale_parse(char *str, int *scale);

edges_t *allocate_edges(int n_bins);
void free_edges(edges_t **edges);
int edges_get_index(edges_t *edges, long long int value);
int edges_from_limits(edges_t *edges, limits_t *limits, int scale);
void print_edges(FILE *out_stream, edges_t *edges);

gn_histogram_t *allocate_gn_histogram(int n_dimensions, edges_t **dimensions);
void free_gn_histogram(gn_histogram_t **histogram);

int gn_histogram_make_n_bins(gn_histogram_t *histogram);
int gn_histogram_make_index_bases(gn_histogram_t *histogram);
int gn_histogram_get_index(gn_histogram_t *histogram, 
		long long int *values);
int gn_histogram_get_index_from_indices(gn_histogram_t *histogram,
		int *indices);
int gn_histogram_increment(gn_histogram_t *histogram,
		long long int *values);
void print_gn_histogram(FILE *out_stream, gn_histogram_t *histogram);
int gn_histogram_next_index(gn_histogram_t *histogram, int *indices);

#endif
