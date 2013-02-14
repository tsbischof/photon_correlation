#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "histogram_photon.h"
#include "correlate_photon.h"
#include "t2.h"
#include "t3.h"
#include "modes.h"
#include "error.h"

edges_t *edges_alloc(size_t const n_bins) {
	edges_t *edges = NULL;

	edges = (edges_t *)malloc(sizeof(edges_t));

	if ( edges == NULL ) {
		return(edges);
	}

	edges->n_bins = n_bins;
	edges->print_label = 0;
	edges->scale = SCALE_UNKNOWN;
	edges->bin_edges = (float64_t *)malloc(sizeof(float64_t)*(n_bins+1));

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

int edges_index_linear(void const *e, int64_t const value) {
	edges_t *edges = (edges_t *)e;
	return(floor(
			(value-edges->limits.lower) / 
			(edges->limits.upper-edges->limits.lower) *
			edges->limits.bins));
}

int edges_index_log(void const *e, int64_t const value) {
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

int edges_index_log_zero(void const *e, int64_t const value) {
	if ( value == 0 ) {
		return(0);
	} else  {
		return(edges_index_log(e, value));
	}
}

int edges_index_bsearch(void const *e, int64_t const value) {
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

values_vector_t *values_vector_alloc(unsigned int const length) {
	values_vector_t *vv = (values_vector_t *)malloc(sizeof(values_vector_t));

	if ( vv == NULL ) {
		return(vv);
	}

	vv->length = length;
	vv->values = (int64_t *)malloc(sizeof(int64_t)*length);
	
	if ( vv->values == NULL ) {
		values_vector_free(&vv);
		return(vv);
	} 

	return(vv);
}

void values_vector_init(values_vector_t *vv) {
	memset(vv->values, 0, sizeof(int64_t)*vv->length);
}

void values_vector_free(values_vector_t **vv) {
	if ( *vv != NULL ) {
		free((*vv)->values); 
		free(*vv);
	}
}

int64_t values_vector_index(values_vector_t const *vv, edges_t ** const edges) {
/* The values vector needs to be transformed into an index by:
 * 1. Extracting the index of each bin based on the edges.
 * 2. Turning those indices into an overall index, as with combinations.
 */
	int i;
	int result;
	int base = 1;
	int64_t index = 0;

	for ( i = vv->length-1; i >= 0; i-- ) {
		index *= base;

		result = edges[i]->get_index(edges[i], vv->values[i]);

		if ( result < 0 ) {
			return(result);
		}

		index += result;
		base *= edges[i]->n_bins;
	}

	return(index);
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


histogram_gn_t *histogram_gn_alloc(options_t const *options) {
	int i;
	histogram_gn_t *hist = NULL;

	hist = (histogram_gn_t *)malloc(sizeof(histogram_gn_t));

	if ( hist == NULL ) {
		return(hist);
	} 

	hist->channels = options->channels;
	hist->order = options->order;
	hist->mode = options->mode;

	hist->time_scale = options->time_scale;
	hist->pulse_scale = options->pulse_scale;

	if ( hist->mode == MODE_T2 ) {
		hist->dimensions = hist->order-1;
		hist->build_channels = t2_correlation_build_channels;
		hist->build_values = t2_correlation_build_values;
	} else if ( hist->mode == MODE_T3 ) {
		if ( hist->order == 1 ) {
			hist->dimensions = 1;
		} else {
			hist->dimensions = (hist->order - 1)*2;
		}

		hist->build_channels = t3_correlation_build_channels;
		hist->build_values = t3_correlation_build_values;
	} else {
		error("Unsupported histogram mode: %d\n", hist->mode);
		histogram_gn_free(&hist);
		return(hist);
	} 

	hist->print = histogram_gn_fprintf;

	hist->edges = (edges_t **)malloc(sizeof(edges_t *)*hist->dimensions);

	if ( hist->edges == NULL ) {
		histogram_gn_free(&hist);
		return(hist);
	}

	if ( hist->mode == MODE_T2 ) {
		for ( i = 0; i < hist->order-1; i++ ) {
			hist->edges[i] = edges_alloc(options->time_limits.bins);
			if ( hist->edges[i] == NULL ) {
				histogram_gn_free(&hist);
				error("Could not allocate edges.\n");
				return(hist);
			}

			if ( edges_init(hist->edges[i], &(options->time_limits),
						options->time_scale, 1) != PC_SUCCESS ) {
				histogram_gn_free(&hist);
				error("Could not initialize edges.\n");
				return(hist);
			}
		}

		hist->n_bins = 
				(size_t)pow_int(options->time_limits.bins, hist->order);
	} else if ( hist->mode == MODE_T3 ) {
		if ( hist->order == 1 ) {
			hist->edges[0] = edges_alloc(options->time_limits.bins);
			if ( hist->edges[0] == NULL ) {
				error("Could not allocate edges.\n");
				histogram_gn_free(&hist);
				return(hist);
			}

			if ( edges_init(hist->edges[0], &(options->time_limits),
					options->time_scale, 0) != PC_SUCCESS ) {
				error("Could not initialize edges.\n");
				histogram_gn_free(&hist);
				return(hist);
			}
			
			hist->n_bins = 
					(size_t)pow_int(options->time_limits.bins, hist->order);
		} else {
			for ( i = 0; i < hist->order-1; i++ ) {
				hist->edges[2*i] = edges_alloc(options->pulse_limits.bins);
				hist->edges[2*i+1] = edges_alloc(options->time_limits.bins);

				if ( hist->edges[2*i] == NULL || hist->edges[2*i+1] == NULL ) {
					error("Could not allocate edges.\n");
					histogram_gn_free(&hist);
					return(hist);
				}

				if ( edges_init(hist->edges[2*i], &(options->pulse_limits),
							options->pulse_scale, 1) != PC_SUCCESS || 
						edges_init(hist->edges[2*i+1], &(options->time_limits),
							options->time_scale, 0) != PC_SUCCESS ) {
					error("Could not initalize edges.\n");
					histogram_gn_free(&hist);
					return(hist);
				}
			}

			hist->n_bins = 
					(size_t)pow_int(options->pulse_limits.bins, hist->order)*
					(size_t)pow_int(options->time_limits.bins, hist->order);
		}
	} else {
		histogram_gn_free(&hist);
		return(hist);
	}

	debug("Histogram has %zu bins.\n", hist->n_bins);
	hist->n_histograms = pow_int(hist->channels, hist->order);

	hist->counts = (uint64_t **)malloc(sizeof(uint64_t *)*hist->n_histograms);

	if ( hist->counts == NULL ) {
		error("Could not allocate histogram bins.\n");
		histogram_gn_free(&hist);
		return(hist);
	}

	for ( i = 0; i < hist->n_histograms; i++ ) {
		hist->counts[i] = (uint64_t *)malloc(sizeof(uint64_t)*hist->n_bins);
		if ( hist->counts[i] == NULL ) {
			error("Could not allocate histogram bins.\n");
			histogram_gn_free(&hist);
			return(hist);
		}
	}

	hist->channels_vector = combination_alloc(hist->order, hist->channels);
	hist->values_vector = values_vector_alloc(hist->dimensions);

	if ( hist->channels_vector == NULL || hist->values_vector == NULL ) {
		error("Could not allocate channels or values vector.\n");
		histogram_gn_free(&hist);
		return(hist);
	}

	hist->edge_indices = edge_indices_alloc(hist->dimensions);
	if ( hist->edge_indices == NULL ) {
		error("Could not allocate edge indices.\n");
		histogram_gn_free(&hist);
		return(hist);
	}

	return(hist);
}

void histogram_gn_init(histogram_gn_t *hist) {
	int i;

	values_vector_init(hist->values_vector);
	combination_init(hist->channels_vector);

	for ( i = 0; i < hist->n_histograms; i++ ) {
		memset(hist->counts[i], 0, sizeof(uint64_t)*hist->n_bins); 
	}
}

void histogram_gn_free(histogram_gn_t **hist) {
	int i;

	if ( *hist != NULL ) {
		combination_free(&((*hist)->channels_vector));
		values_vector_free(&((*hist)->values_vector));
		edge_indices_free(&((*hist)->edge_indices));

		if ( (*hist)->counts != NULL ) {
			for ( i = 0; i < (*hist)->n_histograms; i++ ) {
				free((*hist)->counts[i]);
			}
	
			free((*hist)->counts);
		} 

		if ( (*hist)->edges != NULL ) {
			for ( i = 0; i < (*hist)->dimensions; i++ ) {
				edges_free(&((*hist)->edges[i]));
			} 

			free((*hist)->edges);
		} 

		free(*hist);
	}
}

int histogram_gn_increment(histogram_gn_t *hist, 
		correlation_t const *correlation) {
	int histogram_index;
	int bin_index;

	if ( correlation->order != hist->order ) {
		error("Order of correlation does not match histogram: %d vs. %d\n",
				correlation->order, hist->order);
	}

	hist->build_channels(correlation, hist->channels_vector);
	hist->build_values(correlation, hist->values_vector);

	histogram_index = combination_index(hist->channels_vector);
	bin_index = values_vector_index(hist->values_vector, hist->edges);

	if ( histogram_index < 0 || histogram_index >= hist->n_histograms ) {
		error("Invalid histogram index requested: %d (limit %d). "
				"Check that you have specified the correct number of "
				"channels.\nFailed for channels:\n", 
				histogram_index, hist->n_histograms);
		combination_fprintf(stderr, hist->channels_vector);
		return(PC_ERROR_INDEX);
	}

	if ( bin_index < 0 || bin_index >= hist->n_bins ) {
		error("Invalid bin index requested: %d (limit %d). "
				"Check that the limits given are valid.\n",
				bin_index, hist->n_bins);
		return(PC_ERROR_INDEX);
	}

	debug("Incrementing histogram %d, bin %d\n", histogram_index, bin_index);
	hist->counts[histogram_index][bin_index]++;

	return(PC_SUCCESS);
}
			
int histogram_gn_fprintf(FILE *stream_out, void const *histogram) {
/* Cycle through the combinations of channels, and for each combination
 * of channels cycle over the edges.
 */
	histogram_gn_t *hist = (histogram_gn_t *)histogram;
	int i;
	int histogram_index;
	int channel_index;
	int bin_index;

	combination_init(hist->channels_vector);
	
	while ( combination_next(hist->channels_vector) == PC_SUCCESS ) {
		bin_index = -1;
		histogram_index = combination_index(hist->channels_vector);

		if ( histogram_index >= hist->n_histograms ) {
			error("Trying to print a histogram which does not exist.\n");
			return(PC_ERROR_INDEX);
		}

		edge_indices_init(hist->edge_indices, hist->edges);

		while ( edge_indices_next(hist->edge_indices) == PC_SUCCESS ) {
			bin_index++;
			debug("Yielding bin %d\n", bin_index);

			if ( bin_index >= hist->n_bins ) {
				error("Trying to print a bin that does not exist: %d "
						"(limit %d)\n",
						bin_index, hist->n_bins);
				return(PC_ERROR_INDEX);
			}

			channel_index = 0;

			fprintf(stream_out, "%"PRIu32, 
					hist->channels_vector->values[channel_index++]);

			for ( i = 0; i < hist->dimensions; i++ ) {
				if ( hist->edges[i]->print_label ) {
					fprintf(stream_out, ",%"PRIu32,
							hist->channels_vector->values[channel_index++]);
				}

				fprintf(stream_out, ",%.2"PRIf64",%.2"PRIf64,
						hist->edges[i]->bin_edges[
							hist->edge_indices->values[i]],
						hist->edges[i]->bin_edges[
							hist->edge_indices->values[i]+1]);
			}

			fprintf(stream_out, ",%"PRIu64"\n",
					hist->counts[histogram_index][bin_index]);
		}
	}

	return(PC_SUCCESS);
}

int histogram_photon(FILE *stream_in, FILE *stream_out,
		options_t const *options) {
	int result = PC_ERROR_UNKNOWN;
	histogram_gn_t *hist = NULL;
	correlation_t *correlation = NULL;
	correlation_next_t next;
	correlation_print_t print;

	hist = histogram_gn_alloc(options);
	correlation = correlation_alloc(options->mode, options->order);

	if ( hist == NULL || correlation == NULL ) {
		error("Could not allocate histogram or correlation.\n");
		histogram_gn_free(&hist);
		correlation_free(&correlation);
		return(PC_ERROR_MEM);
	}

	if ( options->mode == MODE_T2 ) {
		next = t2_correlation_fscanf;
		print = t2_correlation_fprintf;
	} else if ( options->mode == MODE_T3 ) {
		next = t3_correlation_fscanf;
		print = t3_correlation_fprintf;
	} else { 
		error("Invalid mode: %d\n", options->mode);
		histogram_gn_free(&hist);
		correlation_free(&correlation);
		return(PC_ERROR_MODE);
	}

	correlation_init(correlation);
	histogram_gn_init(hist);

	while ( next(stream_in, correlation) == PC_SUCCESS ) {
		if ( verbose ) {
			debug("Incrementing for correlation: \n");
			if ( correlation->mode == MODE_T2 ) {
				t2_correlation_fprintf(stderr, correlation);
			} else if ( correlation->mode == MODE_T3 ) {
				t3_correlation_fprintf(stderr, correlation);
			} 
		}
		result = histogram_gn_increment(hist, correlation);
		if ( result != PC_SUCCESS ) {
			error("Could not increment with correlation:\n");
			print(stderr, correlation);
		}
	}

	debug("Finished reading correlations from stream.\n");

	if ( result == PC_SUCCESS ) {
		hist->print(stream_out, hist);
	}

	histogram_gn_free(&hist);
	correlation_free(&correlation); 

	return(PC_SUCCESS);
}
