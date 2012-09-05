#include "histogram_t2.h"

#include "error.h"

int histogram_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	t2_correlation_t *record;
	t2_histograms_t *histograms;
	int result = 0;

	if ( options->order == 1 ) {
		error("Correlation of order 1 is not implemented for t2 mode.\n");
		return(-1);
	}

	record = allocate_t2_correlation(options);
	histograms = make_t2_histograms(options);
	
	if ( record == NULL || histograms == NULL ) {
		error("Could not allocate memory for the histogram run.\n");
		result = -1;
	} else {
		/* Loop through the data. 
		 */
		while ( ! next_t2_correlation(in_stream, record, options) ) {
			if ( verbose ) {
				printf("Found record:\n");
				print_t2_correlation(stderr, record, NEWLINE, options);
			}

			t2_histograms_increment(histograms, record);
		}
	}

	/* We are finished histogramming, print the result. */
	if ( ! result ) {
		print_t2_histograms(out_stream, histograms);
	} else {
		printf("%d\n", result);
	}

	/* Clean up memory. */
	debug("Cleaning up from t2 histogramming.\n");
	free_t2_correlation(&record);
	debug("Freeing histograms\n");
	free_t2_histograms(&histograms);
	debug("Memory freed.\n");
	
	return(result);
}

t2_histograms_t *make_t2_histograms(options_t *options) {
	t2_histograms_t *histograms = NULL;
	int result = 0;
	int i;
	int j;
	edges_t **edges = NULL; /* Scratch space for creating histograms. */

	histograms = (t2_histograms_t *)malloc(sizeof(t2_histograms_t));
	edges = (edges_t **)malloc(sizeof(edges_t *)*options->order-1);

	if ( histograms == NULL || edges == NULL ) {
		result = -1;
	} else {
		histograms->n_histograms = n_combinations(options->channels, 
				options->order);
		histograms->channels = options->channels;
		histograms->order = options->order;
		histograms->edges = (edges_t **)malloc(sizeof(edges_t *)*
				histograms->channels);
		histograms->combination = allocate_combination(histograms->channels,
				histograms->order);
		histograms->current_values = (int64_t *)malloc(
				sizeof(int64_t)*(histograms->order-1));
		histograms->histograms = (gn_histogram_t **)malloc(
				sizeof(gn_histogram_t *)*histograms->n_histograms);

		if ( histograms->edges == NULL 
				|| histograms->combination == NULL 
				|| histograms->current_values == NULL
				|| histograms->histograms == NULL ) {
			error("Could not allocate memory for t2 histograms.\n");
			result = -1;
		} else {
			/* Populate edges. */
			for ( i = 0; !result && i < histograms->channels; i++ ) {
				debug("Creating edges for channel %d.\n", i);
				histograms->edges[i] = allocate_edges(
						options->time_limits.bins);
				if ( histograms->edges[i] == NULL ||
						edges_from_limits(histograms->edges[i], 
							&(options->time_limits), options->time_scale) ) {
					error("Could not create edges for channel %d.\n", i);
					result = -1;
				} else {
					debug("Number of bins: %d.\n", 
							histograms->edges[i]->n_bins);
					sprintf(histograms->edges[i]->dimension_label, "%u", 
							i);
					histograms->edges[i]->print_label = 1;
				} 
			} 
			/* Allocate and populate histograms. */

			for ( i = 0; !result && i < histograms->n_histograms; i++ ) {
				/* Populate the edges for this histogram. */
				for ( j = 1; j < histograms->order; j++ ) {
					edges[j-1] = histograms->edges[
							histograms->combination->digits[j]];
				}  
				
				debug("Creating histogram %d.\n", i);
				histograms->histograms[i] = allocate_gn_histogram(
						histograms->order-1, edges);
				sprintf(histograms->histograms[i]->histogram_label,
						"%d", histograms->combination->digits[0]);
				next_combination(histograms->combination);  
			}   
		}
	}

	if ( result ) {
		free_t2_histograms(&histograms);
		histograms = NULL;
	}

	free(edges);
	
	return(histograms);
}

void free_t2_histograms(t2_histograms_t **histograms) {
	int i;

	if ( *histograms != NULL ) {
		debug("Freeing values buffer.\n");
		free((*histograms)->current_values);
		debug("Freeing combination.\n");
		free_combination(&(*histograms)->combination);

		debug("Freeing edges.\n");
		for ( i = 0; (*histograms)->edges != NULL 
				&& i < (*histograms)->channels; i++ ) {
			free_edges(&((*histograms)->edges[i]));
		}
		free((*histograms)->edges);

		debug("Freeing histogram bins.\n");
		for ( i = 0; (*histograms)->histograms != NULL 
				&& i < (*histograms)->n_histograms; i++ ) {
			free_gn_histogram(&((*histograms)->histograms[i]));
		}
		debug("Freeing histogram bin pointer.\n");
		//free((*histograms)->histograms);
		debug("Freeing the overall histogram.\n");
		free(*histograms);
	}
}

int t2_histograms_increment(t2_histograms_t *histograms,
		t2_correlation_t *record) {
	int result = 0;
	int histogram_index;
	int i;

	/* First, determine the index of the histogram from the channels present.
	 */
	debug("Getting the channels from the record.\n");
	histograms->combination->digits[0] = record->records[0].channel;
	for ( i = 1; i < histograms->order; i++ ) {
		histograms->combination->digits[i] = record->records[i].channel;
	}


	for ( i = 1; i < histograms->order; i++ ) {
		histograms->current_values[i] = record->records[i].time;
	}

	histogram_index = get_combination_index(histograms->combination);

	if ( histogram_index >= 0 && histogram_index < histograms->n_histograms ) {
		debug("Incrementing histogram %d.\n");
		gn_histogram_increment(histograms->histograms[histogram_index], 
				histograms->current_values);
	} else {
		error("Invalid histogram index: %d.\n", histogram_index);
		result = -1;
	}

	return(result);
}

void print_t2_histograms(FILE *out_stream, t2_histograms_t *histograms) {
	int i;
	for ( i = 0; i < histograms->n_histograms; i++ ) {
		print_gn_histogram(out_stream, histograms->histograms[i]);
	}
}

