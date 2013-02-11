#include "intensity_photon.h"
#include "intensity.h"

#include "modes.h"
#include "photon.h"
#include "t2_void.h"
#include "t3_void.h"
#include "error.h"

#define MAX(x,y) (x < y ? y : x);

int intensity_photon(FILE *stream_in, FILE *stream_out, options_t *options) {
	counts_t *counts;
	int result = 0;
	photon_stream_t *photon_stream = photon_stream_alloc(options);
	counts_print_t counts_print = counts_fprintf;
	uint64_t photons_seen = 0;
	uint64_t windows_seen = 0;
	int64_t absolute_lower = 0;

	if ( photon_stream == NULL ) {
		return(PC_ERROR_MEM);
	}

	result = photon_stream_init(photon_stream, stream_in, options);
	if ( result != PC_SUCCESS ) {
		photon_stream_free(&photon_stream);
		return(result);
	}

	counts = counts_alloc(options->channels);

	if ( counts == NULL ) {
		error("Could not allocate counts.\n");
		photon_stream_free(&photon_stream);
		return(PC_ERROR_MEM);
	}

	counts_init(counts);

	while ( pc_check(result = photon_stream_next_photon(photon_stream))  
			== PC_SUCCESS) {
		if ( result == PC_SUCCESS ) {
			debug("Found a photon.\n");
			photons_seen++;
		
			pc_status_print("intensity", photons_seen, options);

			if ( photons_seen == 1 ) {
				absolute_lower = photon_stream->window_dim(
						photon_stream->current_photon);
			}

			counts_increment(counts, 
					photon_stream->channel_dim(photon_stream->current_photon));
		} else if ( result == PC_WINDOW_NEXT ) {
			debug("Next window.\n");
			windows_seen++;

			if ( photons_seen == 0 && !photon_stream->window.set_lower_bound ) {
				/* No photons, and we are waiting until the first to start
				 * emitting the result. 
				 */
			} else {
				if ( windows_seen == 1 && 
						! photon_stream->window.set_lower_bound ) {
					counts->window.lower = absolute_lower;
				} else {
					counts->window.lower = photon_stream->window.limits.lower;
				}

				counts->window.upper = photon_stream->window.limits.upper;
				counts_print(stream_out, counts);
			}

			result = photon_stream_next_window(photon_stream);
			if ( result != PC_SUCCESS ) {
				error("Advancing the window produced an error.\n");
				break;
			}

			counts_init(counts);
		} else {
			error("Unhandled stream state: %d\n", result);
			break;
		}
	}

	/* Print the remaining counts. */
	if ( photons_seen > 0 ) {
		if ( windows_seen == 0 && ! photon_stream->window.set_lower_bound ) {
			counts->window.lower = absolute_lower;
		} else {
			counts->window.lower = photon_stream->window.limits.lower;
		}
		
		if ( options->set_stop_time ) {
			counts->window.upper = photon_stream->window.limits.upper;
		} else {
			counts->window.upper = photon_stream->window_dim(
					photon_stream->current_photon) + 1;
		}

		counts_print(stream_out, counts);
	}

	counts_free(&counts);
	photon_stream_free(&photon_stream);

	if ( result == EOF ) {
		result = PC_SUCCESS;
	}

	return(result);
} 

