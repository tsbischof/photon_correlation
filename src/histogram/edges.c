#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "edges.h"
#include "../error.h"

edges_t *edges_alloc(size_t const n_bins) {
	edges_t *edges = NULL;

	edges = (edges_t *)malloc(sizeof(edges_t));

	if ( edges == NULL ) {
		return(edges);
	}

	edges->n_bins = n_bins;
	edges->print_label = 0;
	edges->scale = SCALE_UNKNOWN;
	edges->bin_edges = (double *)malloc(sizeof(double)*(n_bins+1));

	if ( edges->bin_edges == NULL ) {
		edges_free(&edges);
		return(edges);
	}

	return(edges);
}

int edges_init(edges_t *edges, limits_t const *limits, int const scale, 
		int const print_label) {
	int i;

	edges->scale = scale;
	if ( scale == SCALE_LINEAR ) {
		edges->get_index = edges_index_linear;
	} else if ( scale == SCALE_LOG ) {
		edges->get_index = edges_index_log;
	} else if ( scale == SCALE_LOG_ZERO ) {
		edges->get_index = edges_index_log_zero;
	} else {
		edges->get_index = edges_index_bsearch;
	}

	memcpy(&(edges->limits), limits, sizeof(limits_t));
	if ( limits->bins != edges->n_bins ) {
		return(PC_ERROR_MISMATCH);
	}

	for ( i = 0; i <= edges->n_bins; i++ ) {
		edges->bin_edges[i] = limits->lower + 
				(limits->upper - limits->lower)/limits->bins*i;
	}

	edges->print_label = print_label;

	return(PC_SUCCESS);
}

void edges_free(edges_t **edges) {
	if ( *edges != NULL ) {
		free((*edges)->bin_edges);
		free(*edges);
	}
}

int edges_index_linear(void const *e, long long const value) {
	edges_t *edges = (edges_t *)e;
	return(floor(
			(value-edges->limits.lower) / 
			(edges->limits.upper-edges->limits.lower) *
			edges->limits.bins));
}

int edges_index_log(void const *e, long long const value) {
	edges_t *edges = (edges_t *)e;

	if ( value <= 0 ) {
		return(PC_ERROR_INDEX);
	} else {
		return(floor(
				(log(value)-log(edges->limits.lower)) / 
				(log(edges->limits.upper)-log(edges->limits.lower)) *
				edges->limits.bins));
	}
}

int edges_index_log_zero(void const *e, long long const value) {
	if ( value == 0 ) {
		return(0);
	} else  {
		return(edges_index_log(e, value));
	}
}

int edges_index_bsearch(void const *e, long long const value) {
	/* Perform a binary search of the edges to determine which bin the value
	 * falls into. 
	 */
	edges_t *edges = (edges_t *)e;
	size_t lower_index;
	size_t upper_index;;
	size_t middle_index;

	/* Check that the value lies within the lower and upper limits of 
	 * the bins.
 	 */
	lower_index = 0;
	upper_index = edges->n_bins;

	if ( value < edges->bin_edges[0] ) {
		return(-1);
	} else if ( value > edges->bin_edges[edges->n_bins] ) {
		return(-1);
	}

	while ( upper_index - lower_index > 1) {
		middle_index = (upper_index + lower_index)/2;
		if ( value >= edges->bin_edges[middle_index] ) {
			lower_index = middle_index;
		} else {
			upper_index = middle_index;
		}
	}

	return(lower_index);
}

edge_indices_t *edge_indices_alloc(unsigned int const length) {
	edge_indices_t *edge_indices;

	edge_indices = (edge_indices_t *)malloc(sizeof(edge_indices_t));

	if ( edge_indices == NULL ) {
		return(edge_indices);
	}

	edge_indices->length = length;
	edge_indices->yielded = 0;
	edge_indices->limits = (unsigned int *)malloc(sizeof(unsigned int)*length);
	edge_indices->values = (unsigned int *)malloc(sizeof(unsigned int)*length);

	if ( edge_indices->limits == NULL || edge_indices->values == NULL ) {
		edge_indices_free(&edge_indices);
		return(edge_indices);
	}

	return(edge_indices);
}

void edge_indices_init(edge_indices_t *edge_indices, edges_t ** const edges) {
	int i;

	edge_indices->yielded = 0;

	for ( i = 0; i < edge_indices->length; i++ ) {
		edge_indices->limits[i] = edges[i]->n_bins;
		edge_indices->values[i] = 0;
	}
}

void edge_indices_free(edge_indices_t **edge_indices) {
	if ( *edge_indices != NULL ) {
		free((*edge_indices)->limits);
		free((*edge_indices)->values);
		free(*edge_indices);
	}
}

int edge_indices_next(edge_indices_t *edge_indices) {
	int i;

	if ( edge_indices->yielded ) {
		for ( i = edge_indices->length - 1; i >= 0; i-- ) {
			edge_indices->values[i] = (edge_indices->values[i] + 1) %
					edge_indices->limits[i];
			if ( edge_indices->values[i] != 0 ) {
				i = 0;
			} else if ( i == 0 ) {
				return(PC_COMBINATION_OVERFLOW);
			}
		}

		return(PC_SUCCESS);
	} else {
		edge_indices->yielded = 1;
		return(PC_SUCCESS);
	}
}

