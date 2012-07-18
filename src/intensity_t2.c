#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "intensity_t2.h"
#include "t2.h"

int intensity_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	t2_t record;
	counts_t *counts;
	long long int bin_upper_limit;
	int result = 0;
	int done = 0;

	counts = allocate_counts(options->channels);

	if ( counts == NULL ) {
		result = -1;
	}

	bin_upper_limit = options->bin_width;
	init_counts(counts);

	done = next_t2(in_stream, &record);

	while ( ! result && ! done ) {
		if ( (! options->count_all) && record.time > bin_upper_limit ) {
			print_counts(out_stream, 
					bin_upper_limit-options->bin_width, 
					bin_upper_limit,
					counts);
			bin_upper_limit += options->bin_width;
			init_counts(counts);
		} else {
			increment_counts(counts, record.channel);
			done = next_t2(in_stream, &record);
		}
	}

	print_counts(out_stream, 
			bin_upper_limit - options->bin_width, 
			record.time, 
			counts);

	free_counts(&counts);
	return(0);
}
