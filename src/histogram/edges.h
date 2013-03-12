#ifndef HISTOGRAM_EDGES_H_
#define HISTOGRAM_EDGES_H_

#include "../limits.h"

typedef struct {
	unsigned int n_bins;
	int print_label;
	limits_t limits;
	int scale;
	int (*get_index)(void const *edges, long long const value);
	double *bin_edges;
} edges_t;

edges_t *edges_alloc(size_t const n_bins);
int edges_init(edges_t *edges, limits_t const *limits, int const scale,
		int const print_label);
void edges_free(edges_t **edges);

int edges_index_linear(void const *edges, long long const value);
int edges_index_log(void const *edges, long long const value);
int edges_index_log_zero(void const *edges, long long const value);
int edges_index_bsearch(void const *edges, long long const value);

typedef struct {
	unsigned int length;
	int yielded;
	unsigned int *limits;
	unsigned int *values;
} edge_indices_t;

edge_indices_t *edge_indices_alloc(unsigned int const length);
void edge_indices_init(edge_indices_t *edge_indices, edges_t ** const edges);
void edge_indices_free(edge_indices_t **edge_indices);
int edge_indices_next(edge_indices_t *edge_indices);

#endif
