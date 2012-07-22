#include "intensity_t2.h"
#include "intensity.h"

#include "t2.h"

#include "error.h"

int intensity_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	t2_t record;
	counts_t *counts;
	int result = 0;

	t2_windowed_stream_t stream;

	init_t2_windowed_stream(&stream, in_stream, options);
	counts = allocate_counts(options->channels);

	if ( counts == NULL ) {
		result = -1;
	}

	init_counts(counts);

	while ( ! feof(in_stream) && result >= 0 ) {
		result = next_t2_windowed(&stream, &record, options);

		if ( result == 0 ) {
			/* Photon in window. */
			increment_counts(counts, record.channel);
		} else if ( result > 0 ) {
			/* Next window */
			print_counts(out_stream,
					stream.window.limits.lower,
					stream.window.limits.upper,
					counts);
			next_t2_window(&(stream.window));
			init_counts(counts);
		} else { 
			/* End of the stream, or at least a failure while reading it. */
			if ( result < -1 ) {
				error("Error while processing photon stream.\n");
			}
		}
	}

	print_counts(out_stream, 
			stream.window.limits.lower,
			record.time,
			counts);

	free_counts(&counts);
	return(0);
} 
