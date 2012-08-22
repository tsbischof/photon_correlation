#include "histogram_t3.h"

#include "error.h"

int histogram_t3(FILE *in_stream, FILE *out_stream, options_t *options) {
	if ( options->order == 1 ) {
		debug("Doing first-order correlation.\n");
		histogram_t3_g1(in_stream, out_stream, options);
	} else {
		debug("Doing high-order t3 correlation.\n");
		histogram_t3_gn(in_stream, out_stream, options);
	}

	return(0);
}

int histogram_t3_g1(FILE *in_stream, FILE *out_stream, options_t *options) {
	int result = 0;
	int i;
	gn_histogram_t **histograms = NULL;
	edges_t **edges = NULL;
	t3_t record;
	long long int values[1];

	debug("Allocating memory for the histograms and edges.\n");
	histograms = (gn_histogram_t **)malloc(sizeof(gn_histogram_t *)*
					options->channels);
	edges = (edges_t **)malloc(sizeof(edges_t *)*options->channels);

	if ( histograms == NULL || edges == NULL ) {
		error("Could not allocate memory for the histograms.\n");
		result = -1;
	} else { 
		/* First, allocate the edges for each dimension. */
		debug("Populating the edge definitions.\n");
		for ( i = 0; ! result && i < options->channels; i++ ) {
			debug("Dimension %d.\n", i);
			edges[i] = allocate_edges(options->time_limits.bins);
			if ( edges[i] == NULL ) {
				result = -1;
				i = options->channels;
				error("Could not allocate memory for dimension %d.\n");
			} else {
				debug("Writing edges from limits definition.\n");
				if ( edges_from_limits(edges[i], &(options->time_limits),
						options->time_scale) ) {
					error("Error while creating edges from limits "
							"defintion.\n");
					result = -1;
					i = options->channels;
				}
			}
		}

		debug("Populating the histogram definitions.\n");
		/* Now, build up the histograms from the edge definitions. */
		for ( i = 0; !result && i < options->channels; i++ ) {
			debug("Dimension %d.\n", i);
			histograms[i] = allocate_gn_histogram(options->order, &edges[i]);
			sprintf(histograms[i]->histogram_label, "%d", i);
			if ( histograms[i] == NULL ) {
				error("Could not allocate memory for histogram %d.\n", i);
				result = -1;
				i = options->channels;
			}
		}
	}

	/* Follow the stream and perform the histogramming. */
	while ( !result && !next_t3(in_stream, &record) ) {
		debug("Record: %d,%lld,%d\n", record.channel, record.pulse_number,
				record.time);
		values[0] = record.time;
		if ( record.channel >= options->channels ) {
			error("Found a record with channel number %d, "
					"but %d is the limit.\n", record.channel, 
			options->channels-1);
		} else {
			gn_histogram_increment(histograms[record.channel], &values[0]);
		}
	}

	/* Print the histograms. */
	for ( i = 0; !result && i < options->channels; i++ ) {
		print_gn_histogram(out_stream, histograms[i]);
	}

	/* Cleanup. */
	for ( i = 0; i < options->channels; i++ ) {
		debug("Freeing edges.\n");
		free_edges(&edges[i]);
		debug("Freeing histograms.\n");
		free_gn_histogram(&histograms[i]);
	}
	free(edges);
	free(histograms);

	return(0);
}

/* t3_gn is sufficiently complicated that we actually should break up the 
 * functionality into subroutines.
 */
t3_correlated_t *allocate_t3_correlated(options_t *options) {
	t3_correlated_t *record = NULL;

	record = (t3_correlated_t *)malloc(sizeof(t3_correlated_t));
	if ( record != NULL ) { 
		record->records = (t3_t *)malloc(sizeof(t3_t)*(options->order-1));
		if ( record->records == NULL ) {
			free_t3_correlated(&record);
		}
	}

	return(record);
}

void free_t3_correlated(t3_correlated_t **record) {
	if ( *record != NULL ) {
		if ( (*record)->records != NULL ) {
			free((*record)->records);
		}
		free(*record);
	}
}

int next_t3_correlated(FILE *in_stream, t3_correlated_t *record,
		 options_t *options) {
	int result;
	int i;

	result = (fscanf(in_stream, "%d", &(*record).ref_channel) != 1);

	if ( result && !feof(in_stream) ) {
		error("Could not read reference channel from stream.\n");
	} else {
		for ( i = 0; i < options->order - 1; i++ ) {
			result = ( fscanf(in_stream, ",%d,%lld,%d", 
					&(*record).records[i].channel,
					&(*record).records[i].pulse_number,
					&(*record).records[i].time) != 3);
			if ( result && !feof(in_stream)) {
				error("Could not read correlated record (index %d).\n", i);
				i = options->order;
			}
		}

	}
			
	return(result);
}

int histogram_t3_gn(FILE *in_stream, FILE *out_stream, options_t *options) {
	t3_correlated_t *record;
	t3_histograms_t *histograms;
	int result = 0;
	int i;

	record = allocate_t3_correlated(options);
	histograms = make_t3_histograms(options);
	
	if ( record == NULL || histograms == NULL ) {
		error("Could not allocate memory for the histogram run.\n");
		result = -1;
	} else { 
		/* Loop through the data. 
		 */
		while ( !(result = next_t3_correlated(in_stream, record, options)) ) {
			if ( verbose ) {
				fprintf(out_stream, "Found record: %d", record->ref_channel);
				for ( i = 0; i < options->order-1; i++ ) {
					fprintf(out_stream, ",%d,%lld,%d", 
							record->records[i].channel,
							record->records[i].pulse_number,
							record->records[i].time);
				}
				fprintf(out_stream, "\n");
			}

			t3_histograms_increment(histograms, record);
		}
	}

	/* We are finished histogramming, print the result. */
	if ( ! result ) {
		print_t3_histograms(out_stream, histograms);
	} 

	/* Clean up memory. */
	debug("Cleaning up from t3 histogramming.\n");
	free_t3_correlated(&record);
	free_t3_histograms(&histograms); 
	
	return(result);
}

t3_histograms_t *make_t3_histograms(options_t *options) {
	t3_histograms_t *histograms = NULL;
	int result = 0;
	int i;
	int j;
	edges_t **edges = NULL; /* Scratch space for creating histograms. */
	
	/* This is the same as t2 mode, except that we have double the number of
	 * dimensions (each record carries pulse number and time).
	 *
	 * Effectively, this means we have double the dimensions (edges),
	 * and double the order, so just double those values when allocating
	 * and fill in the meaningful bits in each dimension definition.
	 */

	histograms = (t3_histograms_t *)malloc(sizeof(t3_histograms_t));
	edges = (edges_t **)malloc(sizeof(edges_t *)*2*(options->order-1));

	if ( histograms == NULL || edges == NULL ) {
		result = -1;
	} else {
		histograms->n_histograms = n_combinations(options->channels, 
				options->order);
		histograms->channels = options->channels;
		histograms->order = options->order;
		
		/* Two edge sets per channel: pulse and time. */
		histograms->edges = (edges_t **)malloc(sizeof(edges_t *)*
				2*histograms->channels);

		/* The combinations refer to combinations of channels, so we only
		 * need to allocate for as many as we have channels.
		 */
		histograms->combination = allocate_combination(histograms->channels,
				histograms->order);

		histograms->current_values = (long long int *)malloc(
				sizeof(long long int)*2*(histograms->order-1));

		histograms->histograms = (gn_histogram_t **)malloc(
				sizeof(gn_histogram_t *)*histograms->n_histograms);

		if ( histograms->edges == NULL 
				|| histograms->combination == NULL 
				|| histograms->current_values == NULL
				|| histograms->histograms == NULL ) {
			error("Could not allocate memory for t3 histograms.\n");
			result = -1;
		} else {
			/* Populate edges. */
			for ( i = 0; !result && i < histograms->channels; i++ ) {
				debug("Creating edges for channel %d.\n", i);
				histograms->edges[2*i] = allocate_edges(
						options->pulse_limits.bins);
				histograms->edges[2*i+1] = allocate_edges(
						options->time_limits.bins);
				if ( histograms->edges[2*i] == NULL 
						|| edges_from_limits(histograms->edges[2*i], 
							&(options->pulse_limits), options->pulse_scale) ) {
					error("Could not create edges for the pulse component "
							"of channel %d.\n", i);
					result = -1;
				} else if ( histograms->edges[2*i+1] == NULL 
						|| edges_from_limits(histograms->edges[2*i+1],
							&(options->time_limits), options->time_scale) ) {
					error("Could not create edges for the time component "
							"of channel %d.\n", i);
					result = -1;
				} else {
					debug("Number of pulse bins: %d.\n", 
							histograms->edges[2*i]->n_bins);
					sprintf(histograms->edges[2*i]->dimension_label, "%u", 
							i);
					histograms->edges[2*i]->print_label = 1;

					/* We want the output to look like a t3 run, 
					 * so only print the channel number with the pulse and 
					 * not the time.
					 */
					debug("Number of time bins: %d.\n", 
							histograms->edges[2*i+1]->n_bins);
					histograms->edges[2*i+1]->print_label = 0;
				} 
			} 
			/* Allocate and populate histograms. */

			for ( i = 0; !result && i < histograms->n_histograms; i++ ) {
				/* Populate the edges for this histogram. */
				for ( j = 1; j < histograms->order; j++ ) {
/*					printf("(%d, %d)/%d\n", 2*(j-1), 2*(j-1)+1, 
							2*(histograms->order-1)); */
					edges[2*(j-1)] = histograms->edges[
							2*histograms->combination->digits[j]];
					edges[2*(j-1)+1] = histograms->edges[
							2*histograms->combination->digits[j]+1];
				}  
				
				debug("Creating histogram %d.\n", i);
				histograms->histograms[i] = allocate_gn_histogram(
						2*(histograms->order-1), edges);
				sprintf(histograms->histograms[i]->histogram_label,
						"%u", histograms->combination->digits[0]);
				next_combination(histograms->combination);  
			}   
		}
	}

	if ( result ) {
		free_t3_histograms(&histograms);
		histograms = NULL;
	}

	free(edges);
	
	return(histograms);
}

void free_t3_histograms(t3_histograms_t **histograms) {
	int i;

	if ( *histograms != NULL ) {
		debug("Freeing values buffer.\n");
		free((*histograms)->current_values);
		debug("Freeing combination.\n");
		free_combination(&(*histograms)->combination);

		for ( i = 0; (*histograms)->edges != NULL 
				&& i < 2*(*histograms)->channels; i++ ) {
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

int t3_histograms_increment(t3_histograms_t *histograms,
		t3_correlated_t *record) {
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
		histograms->current_values[2*i] = record->records[i].pulse_number;
		histograms->current_values[2*i+1] = record->records[i].time;
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

void print_t3_histograms(FILE *out_stream, t3_histograms_t *histograms) {
	int i;
	for ( i = 0; i < histograms->n_histograms; i++ ) {
		print_gn_histogram(out_stream, histograms->histograms[i]);
	}
}

