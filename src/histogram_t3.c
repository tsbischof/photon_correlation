#include <stdio.h>

#include "combinations.h"
#include "histogram_gn.h"
#include "histogram_t3.h"
#include "t3.h"
#include "error.h"

int histogram_t3(FILE *in_stream, FILE *out_stream, options_t *options) {
	if ( options->order == 1 ) {
		debug("Doing first-order correlation.\n");
		histogram_t3_g1(in_stream, out_stream, options);
	} else {
		error("High-order t3 correlations not yet implemented.\n");
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

int histogram_t3_gn(FILE *in_stream, FILE *out_stream, options_t *options) {
	return(0);
}
