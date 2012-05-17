#include <stdio.h>

#include "histogram.h"
#include "histogram_t2.h"
#include "t2.h"
#include "error.h"
#include "combinations.h"

t2_correlated_t *allocate_t2_correlated(options_t *options) {
	t2_correlated_t *record = NULL;

	record = (t2_correlated_t *)malloc(sizeof(t2_correlated_t));
	if ( record != NULL ) { 
		record->records = (t2_t *)malloc(sizeof(t2_t)*(options->order-1));
		if ( record->records == NULL ) {
			free_t2_correlated(&record);
		}
	}

	return(record);
}

void free_t2_correlated(t2_correlated_t **record) {
	if ( *record != NULL ) {
		if ( (*record)->records != NULL ) {
			free((*record)->records);
		}
		free(*record);
	}
}

int next_t2_correlated(FILE *in_stream, t2_correlated_t *record,
		 options_t *options) {
	int result;
	int i;

	result = (fscanf(in_stream, "%u", &(*record).ref_channel) != 1);

	if ( result && !feof(in_stream) ) {
		error("Could not read reference channel from stream.\n");
	} else {
		for ( i = 0; i < options->order - 1; i++ ) {
			result = ( fscanf(in_stream, ",%u,%lld", 
					&(*record).records[i].channel,
					&(*record).records[i].time) != 2);
			if ( result && !feof(in_stream)) {
				error("Could not read correlated record (index %d).\n", i);
				i = options->order;
			}
		}

	}
			
	return(result);
}

int histogram_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	t2_correlated_t *record;
	t2_histograms_t *histograms;
	int result = 0;
	int i;

	if ( options->order == 1 ) {
		error("Correlation of order 1 is not implemented for t2 mode.\n");
		return(-1);
	}

	record = allocate_t2_correlated(options);
	histograms = make_t2_histograms(options);
	
	if ( record == NULL || histograms == NULL ) {
		error("Could not allocate memory for the histogram run.\n");
		result = -1;
	} else {
		/* Loop through the data. 
		 */
		while ( !next_t2_correlated(in_stream, record, options) ) {
			if ( verbose ) {
				fprintf(out_stream, "Found record: %u", record->ref_channel);
				for ( i = 0; i < options->order-1; i++ ) {
					fprintf(out_stream, ",%u,%lld", 
							record->records[i].channel,
							record->records[i].time);
				}
				fprintf(out_stream, "\n");
			}

			t2_histograms_increment(histograms, record);
		}
	}

	/* We are finished histogramming, print the result. */
	if ( ! result ) {
		print_t2_histograms(out_stream, histograms);
	}

	/* Clean up memory. */
	debug("Cleaning up from t2 histogramming.\n");
	free_t2_correlated(&record);
	free_t2_histograms(&histograms);
	
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
		histograms->current_values = (long long int *)malloc(
				sizeof(long long int)*(histograms->order-1));
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
						"%u", histograms->combination->digits[0]);
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

		for ( i = 0; (*histograms)->edges != NULL 
				&& i < (*histograms)->channels; i++ ) {
			free_edges(&((*histograms)->edges[i]));
		}
		free((*histograms)->edges);

		for ( i = 0; (*histograms)->histograms != NULL 
				&& i < (*histograms)->n_histograms; i++ ) {
			free_gn_histogram(&((*histograms)->histograms[i]));
		}
		free((*histograms)->histograms);
		free(*histograms);
	}
}

int t2_histograms_increment(t2_histograms_t *histograms,
		t2_correlated_t *record) {
	int result = 0;
	int histogram_index;
	int i;

	/* First, determine the index of the histogram from the channels present.
	 */
	debug("Getting the channels from the record.\n");
	histograms->combination->digits[0] = record->ref_channel;
	for ( i = 1; i < histograms->order; i++ ) {
		histograms->combination->digits[i] = record->records[i-1].channel;
	}


	for ( i = 0; i < histograms->order-1; i++ ) {
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

