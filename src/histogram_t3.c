#include "histogram_t3.h"

#include "error.h"

int histogram_t3(FILE *stream_in, FILE *stream_out, options_t *options) {
	if ( options->order == 1 ) {
		debug("Doing first-order correlation.\n");
		histogram_t3_g1(stream_in, stream_out, options);
	} else {
		debug("Doing high-order t3 correlation.\n");
		histogram_t3_gn(stream_in, stream_out, options);
	}

	return(0);
}

int histogram_t3_g1(FILE *stream_in, FILE *stream_out, options_t *options) {
	int result = 0;
	int i;
	gn_histogram_t **histograms = NULL;
	edges_t **edges = NULL;
	t3_t record;
	int64_t values[1];

	/* This process represents a histogram of all of the arrival times
 	 * on each of the channels. It is a three-step process:
 	 * 1. Allocate a histogram for each channel.
 	 * 2. For each record, increment the corresponding bin in the corresponding
 	 *    histogram.
 	 * 3. Print all histograms.
 	 *
 	 * These could be split into subroutines, but the process is sufficiently
 	 * simple that they are not.
 	 */
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
	while ( !result && !next_t3(stream_in, &record, options) ) {
		debug("Record: %d,%"PRIf64",%d\n", record.channel, record.pulse,
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
		print_gn_histogram(stream_out, histograms[i], options);
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

int histogram_t3_gn(FILE *stream_in, FILE *stream_out, options_t *options) {
	t3_correlation_t *correlation;
	t3_histograms_t *histograms;
	int result = 0;

	correlation = allocate_t3_correlation(options);
	histograms = make_t3_histograms(options);
	
	if ( correlation == NULL || histograms == NULL ) {
		error("Could not allocate memory for the histogram run.\n");
		result = -1;
	} else { 
		/* Loop through the data. 
		 */
		while ( !(next_t3_correlation(stream_in, correlation, options)) ) {
			if ( verbose ) {
				fprintf(stderr, "Found record:\n");
				print_t3_correlation(stderr, correlation, NEWLINE, options);
			}

			t3_histograms_increment(histograms, correlation);
		}
	}

	/* We are finished histogramming, print the result. */
	if ( ! result ) {
		print_t3_histograms(stream_out, histograms, options);
	} else {
		error("Error while processing histograms: %d\n", result);
	}

	/* Clean up memory. */
	debug("Cleaning up from t3 histogramming.\n");
	free_t3_correlation(&correlation);
	debug("Freeing histograms.\n");
	free_t3_histograms(&histograms); 
	debug("Memory freed.\n");
	
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

		histograms->current_values = (int64_t *)malloc(
				sizeof(int64_t)*2*(histograms->order-1));

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
		t3_correlation_t *correlation) {
	int result = 0;
	int histogram_index;
	int i;

	/* First, determine the index of the histogram from the channels present.
	 */
	debug("Getting the channels from the record.\n");
	histograms->combination->digits[0] = correlation->records[0].channel;
	for ( i = 1; i < histograms->order; i++ ) {
		histograms->combination->digits[i] = correlation->records[i].channel;
	}


	for ( i = 1; i < histograms->order; i++ ) {
		histograms->current_values[2*(i-1)] = correlation->records[i].pulse;
		histograms->current_values[2*(i-1)+1] = correlation->records[i].time;
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

void print_t3_histograms(FILE *stream_out, t3_histograms_t *histograms,
		options_t *options) {
	int i;
	for ( i = 0; i < histograms->n_histograms; i++ ) {
		print_gn_histogram(stream_out, histograms->histograms[i], options);
	}
}

