#include <stdio.h>

#include "histogram.h"
#include "histogram_t3_g1.h"
#include "t3.h"
#include "error.h"

int histogram_t3_g1(FILE *in_stream, FILE *out_stream, options_t *options) {
	int result = 0;
	t3_t record;
	g1_histograms_t *histograms;
	
	/* Allocate histogram */
	histograms = allocate_g1_histograms(&(options->time_limits), 
			options->channels);	

	if ( histograms == NULL ) {
		error("Could not allocate memory for histograms.\n");
		result = -1;
	} else {
		/* Loop over values. */
		while ( ! next_t3(in_stream, &record) ) {
			increment_g1_histograms(histograms, record.channel, record.time);
		}
		
		print_g1_histograms(out_stream, histograms);
	}

	/* Clean up. */
	free_g1_histograms(&histograms);

	return(result);
}
