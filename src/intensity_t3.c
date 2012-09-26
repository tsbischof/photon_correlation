#include "intensity_t3.h"
#include "intensity.h"

#include "t3.h"

#include "error.h"

int intensity_t3(FILE *in_stream, FILE *out_stream, options_t *options) {
	t3_t record;
	counts_t *counts;
	int result = 0;
	int processed = 0;

	t3_windowed_stream_t stream;

	debug("Initializing windowed stream.\n");
	result = init_t3_windowed_stream(&stream, in_stream, options);

	debug("Initializing counts.\n");
	counts = allocate_counts(options->channels);

	if ( result || counts == NULL ) {
		result = -1;
	}

	init_counts(counts);

	while ( ! feof(in_stream) && result >= 0 ) {
		result = next_t3_windowed(&stream, &record, options);
		if ( result == 0 ) {
			/* Photon in window. */
			processed = 1;
			increment_counts(counts, record.channel);
		} else if ( result == 1 ) {
			/* Next window */
			print_counts(out_stream,
					stream.window.limits.lower,
					stream.window.limits.upper,
					counts);
			next_t3_window(&(stream.window));
			init_counts(counts);
		} else if ( result == 2 ) {
			/* Result is too early to process. */
			;
		} else { 
			/* End of the stream, or at least a failure while reading it. */
			if ( result < -1 ) {
				error("Error while processing photon stream.\n");
				processed = 0;
			}
		}
	}

	/* Use the arrival time of the final photon, unless we have specified some
 	 * starting or stopping time.
 	 */
	if ( processed ) { 
		if ( options->set_stop_time ) {
			print_counts(out_stream,
					stream.window.limits.lower,
					stream.window.limits.upper,
					counts);
		} else {
			print_counts(out_stream, 
					stream.window.limits.lower,
					record.pulse,
					counts);
		}
	}

	free_counts(&counts);
	return(0);
} 
