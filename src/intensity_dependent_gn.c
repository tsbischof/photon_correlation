/*
Copyright (c) 2011-2014, Thomas Bischof
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. Neither the name of the Massachusetts Institute of Technology nor the 
   names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
 */

#include "intensity_dependent_gn.h"
#include "photon/stream.h"
#include "error.h"
#include "modes.h"
#include "photon/queue.h"

int intensity_dependent_gn(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;

	photon_stream_t *photons = NULL;
	idgn_t *idgn = NULL;

	if ( options->window_width == 0 ) {
		error("Window width must be greater than 0.\n");
		return(PC_ERROR_OPTIONS);
	}

	photons = photon_stream_alloc(options->mode);
	idgn = idgn_alloc(options->mode, options->order, options->channels,
			options->queue_size,
			&(options->time_limits), &(options->pulse_limits),
			&(options->intensity_limits));

	if ( photons == NULL || idgn == NULL ) {
		error("Could not allocate photon stream or idgn.\n");
		result = PC_ERROR_MEM;
	} else {
		debug("Initializing\n");
		photon_stream_init(photons, stream_in);
		photon_stream_set_unwindowed(photons);
		idgn_init(idgn, options->window_width);
	}

	debug("Working on stream\n");
	while ( result == PC_SUCCESS && 
			photons->photon_stream_next(photons) == PC_SUCCESS ) {
		result = idgn_push(idgn, &(photons->photon));
	}

	if ( result == PC_SUCCESS ) {
		idgn_flush(idgn);
		idgn_fprintf(stream_out, idgn);
	}
	
	photon_stream_free(&photons);
	idgn_free(&idgn);

	return(result);
}

idgn_t *idgn_alloc(int const mode, int const order, int const channels,
			size_t const queue_size, 
			limits_t const *time_limits, limits_t const *pulse_limits,
			limits_int_t const *intensity_limits) {
	unsigned int i;
	idgn_t *idgn = NULL;

	idgn = (idgn_t *)malloc(sizeof(idgn_t));

	if ( idgn != NULL ) {
		idgn->intensity_bins = intensity_limits->bins;
		idgn->window_width = 0;

		if ( mode == MODE_T2 ) {
			idgn->window_dim = t2_window_dimension;
		} else if ( mode == MODE_T3 ) {
			idgn->window_dim = t3_window_dimension;
		} else {
			error("Mode not supported: %d\n", mode);
			idgn_free(&idgn);
		}
	}

	if ( idgn != NULL ) {
		idgn->photon_queue = photon_queue_alloc(mode, queue_size);

		if ( idgn->photon_queue == NULL ) {
			idgn_free(&idgn);
		}
	}

	if ( idgn != NULL ) {
		idgn->gns = (photon_gn_t **)malloc(sizeof(photon_gn_t *)*
				idgn->intensity_bins);

		if ( idgn->gns == NULL ) {
			idgn_free(&idgn);
		} else {
			for ( i = 0; idgn != NULL && i < idgn->intensity_bins; i++ ) {
				idgn->gns[i] = photon_gn_alloc(mode, order,
						channels, queue_size,
						time_limits, pulse_limits);

				if ( idgn->gns[i] == NULL ) {
					idgn_free(&idgn);
				}
			}
		}
	}

	if ( idgn != NULL ) {
		idgn->windows_seen = counts_alloc(idgn->intensity_bins);
	}

	if ( idgn != NULL ) {
		idgn->intensities = edges_int_alloc(intensity_limits->bins);
	
		if ( idgn->intensities == NULL ) {
			idgn_free(&idgn);
		} else {
			if ( edges_int_init(idgn->intensities, intensity_limits) 
					!= PC_SUCCESS ) {
				idgn_free(&idgn);
			}
		}
	}

	return(idgn);
}

void idgn_init(idgn_t *idgn, long long const window_width) {
	int i;

	photon_window_init(&(idgn->window), window_width, false, 0, false, 0);

	queue_init(idgn->photon_queue);

	for ( i = 0; i < idgn->intensity_bins; i++ ) {
		photon_gn_init(idgn->gns[i]);
	}

	counts_init(idgn->windows_seen);
}

int idgn_push(idgn_t *idgn, photon_t const *photon) {
	int status = PC_SUCCESS;

	while ( true ) {
		status = photon_window_contains(&(idgn->window), 
				idgn->window_dim(photon));

		if ( status == PC_RECORD_IN_WINDOW ) {
			queue_push(idgn->photon_queue, photon);
			return(PC_SUCCESS);
		} else if ( status == PC_RECORD_AFTER_WINDOW ) {
			idgn_update(idgn);
			photon_window_next(&(idgn->window));
		} else {
			error("Photon stream does not appear to be time-ordered.\n");
			return(PC_RECORD_BEFORE_WINDOW);
		}
	}
}

int idgn_update(idgn_t *idgn) {
	/* Take the current accumulated photons and add them to the appropriate
	 * queue. We do not flush the gn, so we can use counts from consecutive
	 * time windows.
	 */
	int result = PC_SUCCESS;
	size_t counts = queue_size(idgn->photon_queue);
	photon_t *photon;
	int index = edges_int_index_linear(idgn->intensities, counts);

	if ( 0 <= index && index < idgn->intensity_bins ) {
		debug("Updating index %d (%zu counts)\n", index, counts);

		counts_increment(idgn->windows_seen, index);

		while ( queue_size(idgn->photon_queue) > 0 ) {
			queue_front(idgn->photon_queue, (void *)&photon);
			photon_gn_push(idgn->gns[index], photon);
			queue_pop(idgn->photon_queue, NULL);
		}
	} else {
		error("Invalid intensity: %zu found, limits are (%lld, %lld)\n",
				counts,
				idgn->intensities->limits.lower,
				idgn->intensities->limits.upper);

		queue_init(idgn->photon_queue);

		result = PC_ERROR_INDEX;
	}

	return(result);
}

int idgn_flush(idgn_t *idgn) {
	int i;
	int result = PC_SUCCESS;

	result = idgn_update(idgn);

	for ( i = 0; i < idgn->intensity_bins; i++ ) {
		photon_gn_flush(idgn->gns[i]);
	}

	return(result);
}

int idgn_fprintf(FILE *stream_out, idgn_t const *idgn) {
	unsigned int i;

	photon_gn_fprintf_bins(stream_out, idgn->gns[0], 4);

	for ( i = 0; i < idgn->intensity_bins; i++ ) {
		fprintf(stream_out, "%lld,%lld,",
				idgn->intensities->bin_edges[i],
				idgn->intensities->bin_edges[i+1]);
		fprintf(stream_out, "%lld,", idgn->window.width);
		fprintf(stream_out, "%llu,", idgn->windows_seen->counts[i]);

		photon_gn_fprintf_counts(stream_out, idgn->gns[i]);
	}
	
	return(ferror(stream_out) ? PC_ERROR_IO : PC_SUCCESS);
}

void idgn_free(idgn_t **idgn) {
	unsigned int i;

	if ( *idgn != NULL ) {
		if ( (*idgn)->gns != NULL ) {
			for ( i = 0; i < (*idgn)->intensity_bins; i++ ) {
				photon_gn_free(&((*idgn)->gns[i]));
			}

			free((*idgn)->gns);
		}

		queue_free(&((*idgn)->photon_queue));

		counts_free(&((*idgn)->windows_seen));
		edges_int_free(&((*idgn)->intensities));

		free(*idgn);
		*idgn = NULL;
	}
}
