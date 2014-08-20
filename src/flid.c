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

#include <stdlib.h>
#include <string.h>
#include "flid.h"
#include "error.h"
#include "modes.h"
#include "photon/stream.h"

int flid(FILE *stream_in, FILE *stream_out, pc_options_t const *options) {
	int status = PC_SUCCESS;
	photon_stream_t *photon_stream = NULL;
	flid_t *flid = NULL;

	if ( options->window_width == 0 ) {
		error("Must specify --window-width for flid.\n");
		status = PC_ERROR_OPTIONS;
	}

	if ( status == PC_SUCCESS ) {
		photon_stream = photon_stream_alloc(MODE_T3);
		flid = flid_alloc(&(options->time_limits), 
				&(options->intensity_limits));

		if ( photon_stream == NULL || flid == NULL ) {
			error("Could not allocate photon stream or flid.\n");
			status = PC_ERROR_MEM;
		}
	}

	if ( status == PC_SUCCESS ) {
		photon_stream_init(photon_stream, stream_in);
		photon_stream_set_unwindowed(photon_stream);

		flid_init(flid, options->window_width);

		while ( status == PC_SUCCESS && 
				photon_stream_next_photon(photon_stream) == PC_SUCCESS ) {
			status = flid_push(flid, &(photon_stream->photon.t3));
		}

		flid_flush(flid);
	}

	if ( status == PC_SUCCESS ) {
		flid_fprintf(stream_out, flid);
	}

	flid_free(&flid);

	return(status);
}

flid_t *flid_alloc(limits_t const *time_limits, 
		limits_int_t const *intensity_limits) {
	flid_t *flid = NULL;
	int i;

	flid = (flid_t *)malloc(sizeof(flid_t));

	if ( flid == NULL ) {
		return(flid);
	} 

	flid->time_axis = edges_alloc(time_limits->bins);
	flid->intensity_axis = edges_int_alloc(intensity_limits->bins);

	if ( flid != NULL ) {
		if ( edges_init(flid->time_axis, time_limits, 
				SCALE_LINEAR, false) != PC_SUCCESS ||
			edges_int_init(flid->intensity_axis, intensity_limits) 
				!= PC_SUCCESS ) {
			flid_free(&flid);
		}
	}

	if ( flid != NULL ) {
		flid->counts = (unsigned int **)malloc(
				sizeof(unsigned int *)*flid->intensity_axis->n_bins);
	}

	if ( flid->counts == NULL ) {
		flid_free(&flid);
	} else {
		for ( i = 0; flid != NULL && i < flid->intensity_axis->n_bins; i++ ) {
			flid->counts[i] = (unsigned int *)malloc(
					sizeof(unsigned int)*flid->time_axis->n_bins);

			if ( flid->counts[i] == NULL ) {
				flid_free(&flid);
			}
		}
	}

	return(flid);
}

void flid_init(flid_t *flid, long long const window_width) {
	int i;

	photon_window_init(&(flid->window), window_width, false, 0, false, 0);
	
	for ( i = 0; i < flid->intensity_axis->n_bins; i++ ) {
		memset(flid->counts[i], 0, 
				sizeof(unsigned int)*flid->time_axis->n_bins);
	}

	flid->total_counts = 0;
	flid->total_lifetime = 0;
}

int flid_push(flid_t *flid, t3_t const *photon) {
	int status;

	while ( true ) {
		status = photon_window_contains(&(flid->window), photon->pulse);

		if ( status == PC_RECORD_IN_WINDOW ) {
			flid->total_counts++;
			flid->total_lifetime += photon->time;
			return(PC_SUCCESS);
		} else if ( status == PC_RECORD_AFTER_WINDOW ) {
			flid_flush(flid);
			photon_window_next(&(flid->window));
		} else {
			error("Photon stream does not appear to be time-ordered.\n");
			error("Expected a pulse between %lld and %lld, but found %lld.\n",
					flid->window.lower_bound, flid->window.upper_bound,
					photon->pulse);

			return(PC_RECORD_BEFORE_WINDOW);
		}
	}
}

int flid_flush(flid_t *flid) {
	int result = PC_SUCCESS;
	double intensity = flid->total_counts;
	double mean_lifetime;

	if ( flid->total_counts != 0 ) {
		mean_lifetime = (double)flid->total_lifetime / 
			(double)flid->total_counts;

		result = flid_update(flid, intensity, mean_lifetime);
	}	

	flid->total_counts = 0;
	flid->total_lifetime = 0;

	return(result);
}

int flid_update(flid_t *flid, long long const intensity, 
		double const lifetime) {
	int time_index = edges_index_linear_double(flid->time_axis,
			lifetime);
	int intensity_index = edges_int_index_linear(flid->intensity_axis, 
			intensity);

	if ( intensity_index < flid->intensity_axis->n_bins 
			&& intensity_index >= 0 ) {
		if ( time_index < flid->time_axis->n_bins 
				&& time_index >= 0 ) {
			flid->counts[intensity_index][time_index]++;
			return(PC_SUCCESS);
		} else {
			error("Found a lifetime out of bounds: got %lf, but expected a "
				"value between %lf and %lf.\n", 
				lifetime, flid->time_axis->limits.lower, 
				flid->time_axis->limits.upper);
			return(PC_ERROR_INDEX);
		}
	} else {
		error("Found an intensity out of bounds: got %lld, but expected a "
				"value between %lld and %lld.\n", 
				intensity, flid->intensity_axis->limits.lower, 
				flid->intensity_axis->limits.upper);
		return(PC_ERROR_INDEX);
	}
}

int flid_fprintf(FILE *stream_out, flid_t const *flid) {
	int i, j, offset;

	/* Lifetime axis is horizontal, intensity is vertical. First two entries
	 * of each row or column are the bin definition.
	 */

	for ( offset = 0; offset < 2; offset++ ) {
		fprintf(stream_out, ",,,");

		for ( i = offset; i < flid->time_axis->n_bins+offset-1; i++ ) {
			fprintf(stream_out, "%lf,", flid->time_axis->bin_edges[i]);
		}
		fprintf(stream_out, "%lf\n", flid->time_axis->bin_edges[i]);
	}

	for ( i = 0; i < flid->intensity_axis->n_bins; i++ ) {
		fprintf(stream_out, "%lld,%lld,", 
				flid->intensity_axis->bin_edges[i],
				flid->intensity_axis->bin_edges[i+1]);

		if ( i == 0 ) {
			fprintf(stream_out, "%lld,", flid->window.width);
		} else {
			fprintf(stream_out, ",");
		}

		for ( j = 0; j < flid->time_axis->n_bins-1; j++ ) {
			fprintf(stream_out, "%u,", flid->counts[i][j]);
		}

		fprintf(stream_out, "%u\n", flid->counts[i][j]);
	}

	return(PC_SUCCESS);
}

void flid_free(flid_t **flid) {
	int i;

	if ( *flid != NULL ) {
		if ( (*flid)->intensity_axis != NULL ) {
			if ( (*flid)->counts != NULL ) {
				for ( i = 0; i < (*flid)->intensity_axis->n_bins; i++ ) {
					free((*flid)->counts[i]);
				}

				free((*flid)->counts);
			}
		}

		edges_free(&((*flid)->time_axis));
		edges_int_free(&((*flid)->intensity_axis));

		free(*flid);
		*flid = NULL;
	}
}
