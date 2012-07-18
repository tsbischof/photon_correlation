#include "intensity_t3.h"
#include "intensity.h"

#include "t3.h"

#include "error.h"

int intensity_t3(FILE *in_stream, FILE *out_stream, options_t *options) {
	t3_t record;
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

	done = next_t3(in_stream, &record);

	while ( ! result && ! done ) {
		if ( (! options->count_all) && record.pulse_number > bin_upper_limit ) {
			print_counts(out_stream, 
					bin_upper_limit-options->bin_width, 
					bin_upper_limit,
					counts);
			bin_upper_limit += options->bin_width;
			init_counts(counts);
		} else {
			increment_counts(counts, record.channel);
			done = next_t3(in_stream, &record);
		}
	}

	print_counts(out_stream, 
			bin_upper_limit - options->bin_width,
			record.pulse_number, 
			counts);

	free_counts(&counts);
	return(0);
}
