#include "intensity_t2.h"
#include "intensity.h"

#include "t2.h"

#include "error.h"

int intensity_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	t2_t record;
	counts_t *counts;
	int result = 0;
	int processed = 0;

	t2_windowed_stream_t stream;

	result = init_t2_windowed_stream(&stream, in_stream, options);
	counts = allocate_counts(options->channels);

	if ( result || counts == NULL ) {
		result = -1;
	}

	init_counts(counts);

	while ( ! feof(in_stream) && result >= 0 ) {
		result = next_t2_windowed(&stream, &record, options);
		if ( result == 0 ) {
			/* Photon in window. */
			processed = 1;
			increment_counts(counts, record.channel);
		} else if ( result == 1 ) {
			/* Next window */
			print_counts(out_stream,
					stream.window.limits.lower,
					stream.window.limits.upper,
					counts,
					options);
			next_t2_window(&(stream.window));
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
					counts,
					options);
		} else {
			print_counts(out_stream, 
					stream.window.limits.lower,
					record.time,
					counts,
					options);
		}
	}

	free_counts(&counts);
	return(0);
} 
