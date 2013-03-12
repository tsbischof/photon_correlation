#include <stdlib.h>
#include <string.h>

#include "histogram_gn.h"
#include "photon.h"
#include "edges.h"
#include "../modes.h"
#include "../error.h"

histogram_gn_t *histogram_gn_alloc(int const mode, unsigned int const order,
		unsigned int const channels, 
		int const time_scale, limits_t const *time_limits,
		int const pulse_scale, limits_t const *pulse_limits) {
	int i;
	histogram_gn_t *hist = NULL;

	hist = (histogram_gn_t *)malloc(sizeof(histogram_gn_t));

	if ( hist == NULL ) {
		return(hist);
	} 

	hist->channels = channels;
	hist->order = order;
	hist->mode = mode;

	hist->time_scale = time_scale;
	hist->pulse_scale = pulse_scale;

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
			hist->edges[i] = edges_alloc(time_limits->bins);
			if ( hist->edges[i] == NULL ) {
				histogram_gn_free(&hist);
				error("Could not allocate edges.\n");
				return(hist);
			}

			if ( edges_init(hist->edges[i], time_limits, time_scale, 1) 
					!= PC_SUCCESS ) {
				histogram_gn_free(&hist);
				error("Could not initialize edges.\n");
				return(hist);
			}
		}

		hist->n_bins = 
				(size_t)pow_int(time_limits->bins, hist->order);
	} else if ( hist->mode == MODE_T3 ) {
		if ( hist->order == 1 ) {
			hist->edges[0] = edges_alloc(time_limits->bins);
			if ( hist->edges[0] == NULL ) {
				error("Could not allocate edges.\n");
				histogram_gn_free(&hist);
				return(hist);
			}

			if ( edges_init(hist->edges[0], time_limits, time_scale, 0) 
					!= PC_SUCCESS ) {
				error("Could not initialize edges.\n");
				histogram_gn_free(&hist);
				return(hist);
			}
			
			hist->n_bins = (size_t)pow_int(time_limits->bins, hist->order);
		} else {
			for ( i = 0; i < hist->order-1; i++ ) {
				hist->edges[2*i] = edges_alloc(pulse_limits->bins);
				hist->edges[2*i+1] = edges_alloc(time_limits->bins);

				if ( hist->edges[2*i] == NULL || hist->edges[2*i+1] == NULL ) {
					error("Could not allocate edges.\n");
					histogram_gn_free(&hist);
					return(hist);
				}

				if ( edges_init(hist->edges[2*i], pulse_limits, pulse_scale, 1)
							!= PC_SUCCESS || 
						edges_init(hist->edges[2*i+1], time_limits,
							time_scale, 0) != PC_SUCCESS ) {
					error("Could not initalize edges.\n");
					histogram_gn_free(&hist);
					return(hist);
				}
			}

			hist->n_bins = 
					(size_t)pow_int(pulse_limits->bins, hist->order)*
					(size_t)pow_int(time_limits->bins, hist->order);
		}
	} else {
		histogram_gn_free(&hist);
		return(hist);
	}

	debug("Histogram has %zu bins.\n", hist->n_bins);
	hist->n_histograms = pow_int(hist->channels, hist->order);

	hist->counts = (unsigned long long **)malloc(
			sizeof(unsigned long long *)*hist->n_histograms);

	if ( hist->counts == NULL ) {
		error("Could not allocate histogram bins.\n");
		histogram_gn_free(&hist);
		return(hist);
	}

	for ( i = 0; i < hist->n_histograms; i++ ) {
		hist->counts[i] = (unsigned long long *)malloc(
				sizeof(unsigned long long)*hist->n_bins);
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
		memset(hist->counts[i], 0, sizeof(unsigned long long)*hist->n_bins); 
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

				fprintf(stream_out, ",%.2lf,%.2lf",
						hist->edges[i]->bin_edges[
							hist->edge_indices->values[i]],
						hist->edges[i]->bin_edges[
							hist->edge_indices->values[i]+1]);
			}

			fprintf(stream_out, ",%llu\n",
					hist->counts[histogram_index][bin_index]);
		}
	}

	return(PC_SUCCESS);
}

