/*
 * Copyright (c) 2011-2014, Thomas Bischof
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Massachusetts Institute of Technology nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>

#include "intensity.h"

#include "../modes.h"
#include "../photon/t2.h"
#include "../photon/t3.h"
#include "../photon/stream.h"
#include "../error.h"

/*
 * Due to the need to have a system where a photon stream can be analyzed in
 * a single pass, the intensity object has to operate in a somewhat 
 * counterintuiive fashion.
 *
 * A normal intensity procedure would look like:
 *
 * window = Window()
 * counts = Counts()
 *
 * for photon in photons:
 *     if photon in window:
 *         counts.increment(photon)
 *     elif photon before window:
 *         continue
 *     else:
 *         print(counts)
 *         counts.init()
 *         window.next()
 *
 * if not counts.empty():
 *     print(counts)
 *
 *
 * To perform this same method where photons can be added arbitrarily and the
 * result accessed as needed, do the following instead:
 *
 * window = Window()
 * intensity = Intensity()
 *
 * for photon in photons:
 *     intensity.push(photon)
 *     while intensity.available():
 *         print(intensity)
 *         intensity.next()
 *
 * intensity.flush()
 * while intensity.available():
 *     print(intensity)
 *     intensity.next()
 *
 *
 * The mechanisms for checking the validity of photons and incrementing
 * counts remain the same, but we have to keep track of state explicitly in
 * the intensity object, rather than implicitly in the procedure.
 */

intensity_photon_t *intensity_photon_alloc(unsigned int const channels, 
		int const mode) {
	intensity_photon_t *intensity = NULL;

	intensity = (intensity_photon_t *)malloc(sizeof(intensity_photon_t));

	if ( intensity == NULL ) {
		return(intensity);
	}

	if ( mode == MODE_T2 ) {
		intensity->channel_dim = t2_channel_dimension;
		intensity->window_dim = t2_window_dimension;
	} else if ( mode == MODE_T3 ) {
		intensity->channel_dim = t3_channel_dimension;
		intensity->window_dim = t3_window_dimension;
	} else {
		error("Invalid mode: %d\n", mode);
		intensity_photon_free(&intensity);
		return(intensity);
	}

	intensity->channels = channels;
	intensity->counts = counts_alloc(intensity->channels);

	if ( intensity->counts == NULL ) {
		intensity_photon_free(&intensity);
		return(intensity);
	}

	return(intensity);
}

void intensity_photon_init(intensity_photon_t *intensity,
		int count_all,
		long long bin_width,
		int set_lower_bound, long long lower_bound,
		int set_upper_bound, long long upper_bound) {
	intensity->first_photon_seen = false;
	intensity->flushing = false;
	intensity->photon_held = false;
	intensity->record_available = false;
	intensity->last_window_seen = 0;
	intensity->yielded = false;

	intensity_photon_counts_init(intensity);

	intensity->count_all = bin_width == 0 || count_all;

	if ( count_all ) {
		photon_window_init(&(intensity->window), 1,
				0, 0,
				0, 0);
	} else {
		photon_window_init(&(intensity->window), bin_width,
				set_lower_bound, lower_bound,
				set_upper_bound, upper_bound);
	}

	intensity->counts->lower = intensity->window.lower;
	intensity->counts->upper = intensity->window.upper;

	if ( set_lower_bound ) {
		intensity->first_photon_seen = true;
	}

	intensity->next = intensity_photon_next_from_photon;
}

void intensity_photon_counts_init(intensity_photon_t *intensity) {
	debug("Initializing counts.\n");
	counts_init(intensity->counts);
}

void intensity_photon_free(intensity_photon_t **intensity) {
	if ( *intensity != NULL ) {
		counts_free(&(*intensity)->counts);
		free(*intensity);
		*intensity = NULL;
	}
}

int intensity_photon_push(intensity_photon_t *intensity, 
		photon_t const *photon) {
	int result;

	unsigned int channel;
	long long window;

	channel = intensity->channel_dim(photon);
	window = intensity->window_dim(photon);

	debug("Pushing (%u, %lld)\n", channel, window);

	if ( channel >= intensity->channels ) {
		error("Invalid channel: %u\n", channel);
		return(PC_ERROR_CHANNEL);
	}

	intensity->last_window_seen = window;
	result = photon_window_contains(&(intensity->window), window);

	if ( result == PC_RECORD_IN_WINDOW || intensity->count_all ) {
		debug("Record in window, incrementing.\n");
		intensity_photon_increment(intensity, channel);

		if ( ! intensity->first_photon_seen ) {
			intensity->window.lower = window;
			intensity->counts->lower = window;
			intensity->first_photon_seen = true;
		}

		return(PC_SUCCESS);
	} else if ( result == PC_RECORD_BEFORE_WINDOW ) {
		debug("Record before window: %lld < (%lld, %lld).\n", 
				window, intensity->window.lower, intensity->window.upper);
		return(PC_SUCCESS);
	} else if ( result == PC_RECORD_AFTER_WINDOW ) {
		debug("Record after window, yield this one and move on.\n");
		memcpy(&(intensity->photon), photon, sizeof(photon_t));
		intensity->photon_held = true;

		if ( intensity->first_photon_seen ) {
			intensity->record_available = true;
			return(PC_WINDOW_NEXT);
		} else {
			while ( 1 ) {
				photon_window_next(&(intensity->window));
				intensity->counts->lower = intensity->window.lower;
				intensity->counts->upper = intensity->window.upper;

				if ( photon_window_contains(&(intensity->window), window) 
						== PC_SUCCESS ) {
					return(PC_SUCCESS);
				}
			}
		}
	} else {
		error("Unknown push state: %d\n", result);
		return(result);
	}
}

int intensity_photon_increment(intensity_photon_t *intensity, 
		unsigned int const channel ) {
	return(counts_increment(intensity->counts, channel));
}

int intensity_photon_next_from_photon(intensity_photon_t *intensity) {
	int result = PC_SUCCESS;

	if ( intensity->yielded ) {
		debug("Yielded, moving to next window.\n");
		intensity_photon_counts_init(intensity);
		photon_window_next(&(intensity->window));

		intensity->counts->lower = intensity->window.lower;
		intensity->counts->upper = intensity->window.upper;

		intensity->yielded = false;
		intensity->record_available = false;
	}

	if ( intensity->record_available ) {
		debug("Record is still available, yield it.\n");
		intensity->record_available = false;
		intensity->yielded = true;
		return(PC_RECORD_AVAILABLE);
	}

	if ( intensity->photon_held ) {
		debug("Photon held, push.\n");
		result = intensity_photon_push(intensity, &(intensity->photon));

		if ( result == PC_SUCCESS ) {
			debug("Pushed successfully.\n");
			intensity->photon_held = false;

			return(PC_ERROR_NO_RECORD_AVAILABLE);
		} else if ( result == PC_WINDOW_NEXT ) {
			debug("Yield this and move to next window.\n");
			intensity->yielded = true;
			intensity->record_available = false;
			
			return(PC_RECORD_AVAILABLE);
		} else {
			error("Unknown next state: %d\n", result);
			return(result);
		}
	} else {
		debug("No photon held.\n");
		if ( intensity->flushing ) {
			if ( counts_nonzero(intensity->counts) ) {
				intensity->record_available = false;
				intensity->flushing = false;
				intensity->yielded = true;

				intensity->window.upper = intensity->last_window_seen + 1;
				intensity->counts->upper = intensity->window.upper;
				return(PC_RECORD_AVAILABLE);
			} else {
				debug("Flushing, but no photons ever arrived.\n");
				return(PC_ERROR_NO_RECORD_AVAILABLE);
			}
		} else {
			return(PC_ERROR_NO_RECORD_AVAILABLE);
		}
	}
}

int intensity_photon_next_from_stream(intensity_photon_t *intensity) {
	return(intensity_photon_fscanf(intensity->stream_in, intensity));
}

int intensity_photon_next(intensity_photon_t *intensity) {
	return(intensity->next(intensity));
}

void intensity_photon_flush(intensity_photon_t *intensity) {
	intensity->flushing = true;
}

void intensity_photon_init_stream(intensity_photon_t *intensity,
		FILE *stream_in) {
	intensity->stream_in = stream_in;
	intensity->next = intensity_photon_next_from_stream;
}

int intensity_photon_fscanf(FILE *stream_in, intensity_photon_t *intensity) {
	int i;
	size_t n_read;

	n_read = fscanf(stream_in,
			"%lld,%lld",
			&(intensity->counts->lower),
			&(intensity->counts->upper));

	if ( n_read != 2 ) {
		return( feof(stream_in) ? EOF : PC_ERROR_IO );
	}

	for ( i = 0; i < intensity->channels; i++ ) {
		n_read = fscanf(stream_in,
				",%llu",
				&(intensity->counts->counts[i]));

		if ( n_read != 1 ) {
			return( feof(stream_in) ? EOF : PC_ERROR_IO );
		}
	}

	return(PC_SUCCESS);
}

int intensity_photon_fprintf(FILE *stream_out, 
		intensity_photon_t const *intensity) {
	int i;

	fprintf(stream_out, 
			"%lld,%lld",
			intensity->counts->lower,
			intensity->counts->upper);

	if ( ferror(stream_out) ) {
		return(PC_ERROR_IO);
	}

	for ( i = 0; i < intensity->channels; i++ ) {
		fprintf(stream_out,
				",%llu",
				intensity->counts->counts[i]);
	
		if ( ferror(stream_out) ) {
			return(PC_ERROR_IO);
		}
	}

	fprintf(stream_out, "\n");

	return(PC_SUCCESS);
}
	

int intensity_photon(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	intensity_photon_t *intensity;
	photon_stream_t *photon_stream;

	debug("Allocating intensity, photon stream.\n");
	intensity = intensity_photon_alloc(options->channels, options->mode);
	photon_stream = photon_stream_alloc(options->mode);

	if ( intensity == NULL || photon_stream == NULL ) {
		error("Could not allocate intensity or photon stream.\n");
		result = PC_ERROR_MEM;
	} 

	debug("Initializing.\n");
	if ( options->bin_width == 0 ) {
		intensity_photon_init(intensity, 
				options->count_all,
				options->mode == MODE_T2 ? 50000000000 : 100000,
				options->set_start, options->start,
				options->set_stop, options->stop);
	} else {
		intensity_photon_init(intensity, 
				options->count_all,
				options->bin_width,
				options->set_start, options->start,
				options->set_stop, options->stop);
	}

	photon_stream_init(photon_stream, stream_in);

	if ( result == PC_SUCCESS ) {
		while ( photon_stream_next_photon(photon_stream) == PC_SUCCESS ) {
			intensity_photon_push(intensity, &(photon_stream->photon));
	
			while ( intensity_photon_next(intensity) == PC_SUCCESS ) {
				intensity_photon_fprintf(stream_out, intensity);
			}
		}
	
		intensity_photon_flush(intensity);
		while ( intensity_photon_next(intensity) == PC_SUCCESS ) {
			intensity_photon_fprintf(stream_out, intensity);
		}
	}

	debug("Cleaning up.\n");
	intensity_photon_free(&intensity);
	photon_stream_free(&photon_stream);
	return(PC_SUCCESS);
}
	
