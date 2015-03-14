/*
 * Copyright (c) 2011-2015, Thomas Bischof
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

#include <stdlib.h>
#include <string.h>

#include "stream.h"
#include "../error.h"
#include "../types.h"

#include "../modes.h"
#include "t2.h"
#include "t3.h"

/* 
 * There are three main use cases for a photon stream:
 * (1) Bulk transfer. In channels, this is used to read and sort large numbers
 *     of photons rapidly.
 * (2) Photon-by-photon processing. In correlate, this is used to ensure that
 *     no more photons are pulled into memory than necessary.
 * (3) Photon-window processing. In intensity, this is used to provide a simple
 *     way of segregating the photons for counting. 
 * 
 * Given these constraints, (1) can be taken care of with the elementary photon
 * read/write routines, or with bulk equivalents if desired. For (3), a 
 * windowed stream, where the current time limits are tracked and iterated over
 * as needed does a good job of providing an iterable interface for users. 
 * Less obviously, (2) can use the same interface, since the window is now
 * infinite. To avoid unnecessary checks of the window bounds, a separate
 * iteration routine can be used which does not include that logic. 
 * 
 * To implement the stream, we need a few things:
 * (1) An incoming, unformatted stream (FILE *)
 * (2) A window (lower, upper)
 * (3) The last photon seen and a flag indicating whether it has been yielded.
 *
 * (3) is only necessary in the windowed case, since it is possible to reach
 * the end of a time bin before the photon can be emitted (and in fact, this
 * is the whole point of the operation). This is the logic we will be avoiding
 * by using a separate iteration routine in use case (2).
 *
 * The basic algorithm for the photon stream goes as follows:
 * window = next(windows)
 * current = next(photons)
 * yielded = False
 * while (photons is not empty) or (not yielded):
 *     if not yielded:
 *         if current in window:
 *             yield(current)
 *             yielded = True
 *         else:
 *             window = next(windows)
 *     else:
 *         current = next(photons)
 *         yielded = False
 * 
 * The non-windowed version is much simpler:
 * while photons is not empty:
 *     yield(next(photons))
 */
photon_stream_t *photon_stream_alloc(int const mode) {
	photon_stream_t *photons = NULL;

	photons = (photon_stream_t *)malloc(sizeof(photon_stream_t));

	if ( photons == NULL ) {
		return(photons);
	}

	photons->mode = mode;
	if ( mode == MODE_T2 ) {
		photons->photon_next = t2_fscanf;
		photons->photon_print = t2_fprintf;
		photons->window_dim = t2_window_dimension;
		photons->channel_dim = t2_channel_dimension;
	} else if ( mode == MODE_T3 ) {
		photons->photon_next = t3_fscanf;
		photons->photon_print = t3_fprintf;
		photons->window_dim = t3_window_dimension;
		photons->channel_dim = t3_channel_dimension;
	} else {
		error("Invalid mode: %d\n", mode);
		photon_stream_free(&photons);
		return(photons);
	}

	return(photons);	
}

void photon_stream_init(photon_stream_t *photons, FILE *stream_in) {
	photon_stream_set_unwindowed(photons);

	/* If the photon has been yielded, the stream is reset and ready to 
	 * acquire a new one. Start in this state to pick up a new photon.
	 */
	photons->yielded = true;
	photons->stream_in = stream_in;
}

void photon_stream_free(photon_stream_t **photons) {
	if ( *photons != NULL ) {
		free(*photons);
		*photons = NULL;
	}
}

void photon_stream_set_unwindowed(photon_stream_t *photons) {
	photons->photon_stream_next = photon_stream_next_unwindowed;

	photon_window_init(&(photons->window),
			1,
			false, 0,
			false, 0);
}

void photon_stream_set_windowed(photon_stream_t *photons,
		long long const bin_width,
		int const set_lower_bound, const long long lower_bound,
		int const set_upper_bound, const long long upper_bound) {
	photons->photon_stream_next = photon_stream_next_windowed;

	photon_window_init(&(photons->window),
			bin_width,
			set_lower_bound, lower_bound,
			set_upper_bound, upper_bound);
}

int photon_stream_next_windowed(photon_stream_t *photons) {
	long long dim;
	int result;

	while ( 1 ) {
		if ( ! photons->yielded ) {
			/* Photon available, check if it is in the window. */
			dim =  photons->window_dim(&photons->photon);
			result = photon_window_contains(&(photons->window), dim);
			if ( result == PC_SUCCESS ) {
				/* Found a photon, and in the window. */
				photons->yielded = true;
				return(PC_SUCCESS);
			} else if ( photons->window.lower > dim ) {	
				/* Have photon, but window is ahead of it. Ignore this photon
				 * and loop until we find another.
				 * Status would be PC_WINDOW_AHEAD
				 */
				debug("Found a photon, but it was before the window.\n");
				photons->yielded = true;
			} else {
				/* Past the upper bound. */
				if ( photons->window.set_upper_bound && 
						dim >= photons->window.upper_bound ) {
					debug("EOF\n");
					return(EOF);
				} else {
					/* Tell the caller to advance the window. */
					debug("Next window (%d).\n", PC_WINDOW_NEXT);
					return(PC_WINDOW_NEXT);
				}
			}
		} else {
			result = photons->photon_next(photons->stream_in, &photons->photon);

			if ( result == PC_SUCCESS ) {
				/* Found one, loop back to see where it falls. */
				photons->yielded = false;
			} else if ( result == EOF ) {
				return(EOF);
			} else {
				/* An error of some nature. */
				error("Unhandled error while processing photon stream: %d.\n",
						result);
				return(result);
			}
		}
	}
}

int photon_stream_next_unwindowed(photon_stream_t *photons) {
	return(photons->photon_next(photons->stream_in, &photons->photon));
}

int photon_stream_next_window(photon_stream_t *photons) {
	return(photon_window_next(&(photons->window)));
}

int photon_stream_next_photon(photon_stream_t *photon_stream) {
	return(photon_stream->photon_stream_next(photon_stream));
}

int photon_stream_eof(photon_stream_t *photons) {
	return(feof(photons->stream_in));
}
