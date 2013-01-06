#include "intensity_void.h"
#include "intensity.h"

#include "modes.h"
#include "photon.h"
#include "t2_void.h"
#include "t3_void.h"
#include "error.h"

#define MAX(x,y) (x < y ? y : x);

int intensity_void(FILE *stream_in, FILE *stream_out, options_t *options) {
	counts_t *counts;
	int result = 0;
	photon_stream_t photons;
	photon_stream_next_t stream_next;
	photon_window_dimension_t window_dim;
	photon_channel_dimension_t channel_dim;
	photon_next_t photon_next;
	counts_print_t counts_print = COUNTS_PRINT(options->binary_out);
	size_t photon_size;
	uint64_t photons_seen = 0;
	uint64_t windows_seen = 0;
	int64_t absolute_lower = 0;
	void *photon;

	if ( options->mode == MODE_T2 ) {
		debug("void mode t2.\n");
		window_dim = t2v_window_dimension;
		photon_size = sizeof(t2_t);
		photon_next = T2V_NEXT(options->binary_in);
		channel_dim = t2v_channel_dimension;
	} /*else if ( options->mode == MODE_T3 ) {
		dim = t3v_window_dim;
		photon_size = sizeof(t3_t);
		photon_next = T3V_NEXT(options->binary_in);
	} */else {
		error("Unsupported mode: %d\n", options->mode);
		return(PC_ERROR_MODE);
	}

	debug("initalizing photon stream.\n");
	result = photon_stream_init(&photons, 
			&stream_next, 
			window_dim,
			photon_next, 
			photon_size, stream_in, 
			options->set_start_time, options->start_time, 
			options->bin_width,
			options->set_stop_time, options->stop_time);

	if ( pc_check(result) ) {
		error("Error while initializing photon stream: %d\n", result);
		return(result);
	} 

	counts = counts_alloc(options->channels);

	if ( counts == NULL ) {
		error("Could not allocate counts.\n");
		return(PC_ERROR_MEM);
	}

	photon = malloc(sizeof(photon));
	if ( photon == NULL ) {
		counts_free(&counts);
		error("Could not allocate photon.\n");
		return(PC_ERROR_MEM);
	}

	/* Set up and ready to go. */

	counts_init(counts);

	while ( pc_check(result = stream_next(&photons, photon))
			 == PC_SUCCESS ) {
		if ( result == PC_SUCCESS ) {
			/* Found a photon. */
			debug("Found a photon.\n");
			photons_seen++;
		
			pc_status_print("intensity", photons_seen, options);

			if ( photons_seen == 1 ) {
				absolute_lower = window_dim(photon);
			}

			counts_increment(counts, channel_dim(photon));
		} else if ( result == PC_WINDOW_AHEAD ) {
			/* Window is ahead of the photon. Ignore this photon, since we can
			 * advance the photon stream until we reach the window.
			 */ 
			debug("Window is ahead of the current photon, ignoring it.\n");
			photons.yielded_photon = 1;
		} else if ( result == PC_WINDOW_NEXT ) {
			/* Go to the next window. */
			debug("Next window.\n");
			windows_seen++;

			if ( windows_seen == 1 && ! photons.window.set_lower_bound ) {
				counts->window.lower = absolute_lower;
			} else {
				counts->window.lower = photons.window.limits.lower;
			}

			counts->window.upper = photons.window.limits.upper;
			counts_print(stream_out, counts);

			result = photon_stream_next_window(&photons);
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
		counts->window.lower = photons.window.limits.lower;
		
		if ( options->set_stop_time ) {
			counts->window.upper = photons.window.limits.upper;
		} else {
			counts->window.upper = window_dim(photon) + 1;
		}

		counts_print(stream_out, counts);
	}

	debug("Freeing counts.\n");
	counts_free(&counts);
	debug("Freeing photon.\n");
	free(photon);
	debug("Done freeing.\n");

	if ( result == EOF ) {
		result = PC_SUCCESS;
	}

	return(result);
} 

