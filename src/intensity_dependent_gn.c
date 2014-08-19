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
		idgn->current_gn = photon_gn_alloc(mode, order, channels,
					queue_size, time_limits, pulse_limits);

		if ( idgn->current_gn == NULL ) {
			idgn_free(&idgn);
		}
	}

	if ( idgn != NULL ) {
		idgn->histograms = (histogram_gn_t **)malloc(sizeof(histogram_gn_t *)
				*idgn->intensity_bins);

		if ( idgn->histograms == NULL ) {
			idgn_free(&idgn);
		} else {
			for ( i = 0; idgn != NULL && i < idgn->intensity_bins; i++ ) {
				idgn->histograms[i] = histogram_gn_alloc(mode, order,
						channels, 
						SCALE_LINEAR, time_limits, 
						SCALE_LINEAR, pulse_limits);

				if ( idgn->histograms[i] == NULL ) {
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

	photon_gn_init(idgn->current_gn);

	for ( i = 0; i < idgn->intensity_bins; i++ ) {
		histogram_gn_init(idgn->histograms[i]);
	}

	counts_init(idgn->windows_seen);
}

int idgn_push(idgn_t *idgn, photon_t const *photon) {
	int status = PC_SUCCESS;

	while ( true ) {
		status = photon_window_contains(&(idgn->window), 
				idgn->window_dim(photon));

		if ( status == PC_RECORD_IN_WINDOW ) {
			idgn->current_counts++;
			photon_gn_push(idgn->current_gn, photon);
			return(PC_SUCCESS);
		} else if ( status == PC_RECORD_AFTER_WINDOW ) {
			idgn_flush(idgn);
			photon_window_next(&(idgn->window));
		} else {
			error("Photon stream does not appear to be time-ordered.\n");
			return(PC_RECORD_BEFORE_WINDOW);
		}
	}
}

int idgn_flush(idgn_t *idgn) {
	int result = PC_SUCCESS;
	int index;

	if ( idgn->current_counts != 0 ) {
		index = edges_int_index_linear(idgn->intensities, idgn->current_counts);

		photon_gn_flush(idgn->current_gn);

		if ( 0 <= index && index < idgn->intensity_bins ) {
			debug("Updating index %d (%lld counts)\n", 
					index, idgn->current_counts);
			histogram_gn_update(idgn->histograms[index],
					idgn->current_gn->histogram);	
			counts_increment(idgn->windows_seen, index);
		} else {
			error("Invalid intensity: %lld found, limits are (%lld, %lld)\n",
					idgn->current_counts,
					idgn->intensities->limits.lower,
					idgn->intensities->limits.upper);
			result = PC_ERROR_INDEX;
		}
	
		photon_gn_init(idgn->current_gn);
		idgn->current_counts = 0;
	}

	return(result);
}

int idgn_fprintf(FILE *stream_out, idgn_t const *idgn) {
	unsigned int i;

	histogram_gn_fprintf_bins(stream_out, idgn->histograms[0], 3);

	for ( i = 0; i < idgn->intensity_bins; i++ ) {
		fprintf(stream_out, "%lld,%lld,%llu,",
				idgn->intensities->bin_edges[i],
				idgn->intensities->bin_edges[i+1],
				idgn->windows_seen->counts[i]);

		histogram_gn_fprintf_counts(stream_out, idgn->histograms[i]);
	}
	
	return(ferror(stream_out) ? PC_ERROR_IO : PC_SUCCESS);
}

void idgn_free(idgn_t **idgn) {
	unsigned int i;

	if ( *idgn != NULL ) {
		if ( (*idgn)->histograms != NULL ) {
			for ( i = 0; i < (*idgn)->intensity_bins; i++ ) {
				histogram_gn_free(&((*idgn)->histograms[i]));
			}

			free((*idgn)->histograms);
		}

		counts_free(&((*idgn)->windows_seen));
		edges_int_free(&((*idgn)->intensities));

		free(*idgn);
		*idgn = NULL;
	}
}
