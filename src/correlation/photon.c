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

#include "photon.h"
#include "correlator.h"
#include "../error.h"
#include "../modes.h"
#include "../photon/t2.h"
#include "../photon/t3.h"
#include "../photon/stream.h"

int correlate_photon(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	photon_stream_t *photon_stream;
	correlator_t *correlator;

	debug("Allocating correlator, photon stream.\n");
	photon_stream = photon_stream_alloc(options->mode);
	correlator = correlator_alloc(options->mode, options->order,
			options->queue_size, options->positive_only,
			options->min_time_distance,
			options->max_time_distance,
			options->min_pulse_distance,
			options->max_pulse_distance);

	if ( correlator == NULL || photon_stream == NULL ) {
		error("Could not allocate correlator (%p), photon stream (%p).\n",
				correlator, photon_stream);
		result = PC_ERROR_MEM;
	}

	if ( result == PC_SUCCESS ) {
		debug("Initializing correlator, photon stream.\n");
		photon_stream_init(photon_stream, stream_in);
		photon_stream_set_unwindowed(photon_stream);
		correlator_init(correlator);

		debug("Starting calculation.\n");
		while ( photon_stream_next_photon(photon_stream) == PC_SUCCESS ) {
			debug("Pushing photon.\n");
			result = correlator_push(correlator, photon_stream->photon);

			if ( result != PC_SUCCESS ) {
				break;
			}
		
			while ( correlator_next(correlator) == PC_SUCCESS ) {
				debug("Found correlation.\n");
				correlator->correlation_print(stream_out, 
						correlator->correlation);
			}
		}

		if ( result == PC_SUCCESS ) {
			debug("Flushing.\n");
			correlator_flush(correlator);
			debug("Checking for correlations.\n");
			while ( correlator_next(correlator) == PC_SUCCESS ) {
				debug("Found correlation.\n");
				correlator->correlation_print(stream_out,
						correlator->correlation);
			}
		}
	}

	correlator_free(&correlator);
	photon_stream_free(&photon_stream);

	return(result);
}

int correlations_echo(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result;
	correlation_next_t next;
	correlation_print_t print;
	correlation_t *correlation;

	if ( options->mode == MODE_T2 ) {
		next = t2_correlation_fscanf;
		print = t2_correlation_fprintf;
	} else if ( options->mode == MODE_T3 ) {
		next = t3_correlation_fscanf;
		print = t3_correlation_fprintf;
	} else {
		error("Unsupported mode: %d\n", options->mode);
		return(PC_ERROR_MODE);
	}

	correlation = correlation_alloc(options->mode, options->order);

	if ( correlation == NULL ) {
		error("Could not allocate correlation.\n");
		return(PC_ERROR_MEM);
	}

	correlation_init(correlation);

	while ( (result = next(stream_in, correlation)) == PC_SUCCESS ) {
		print(stream_out, correlation);
	}

	correlation_free(&correlation);

	return(PC_SUCCESS);
}

void t2_correlate(correlation_t *correlation) {
	int i;

	for ( i = 1; i < correlation->order; i++ ) {
		((t2_t *)correlation->photons)[i].time -=
				((t2_t *)correlation->photons)[0].time;
	}

	if ( correlation->order != 1 ) {
		((t2_t *)correlation->photons)[0].time = 0;
	}
}

int t2_correlation_fscanf(FILE *stream_in, correlation_t *correlation) {
	int i;
	int result;

	result = fscanf(stream_in,
			"%u",
			&(((t2_t *)correlation->photons)[0].channel));

	result = (result == 1);

	((t2_t *)(correlation->photons))[0].time = 0;

	for ( i = 1; result && i < correlation->order; i++ ) {
		result = fscanf(stream_in,
				",%u,%lld",
				&(((t2_t *)correlation->photons)[i].channel),
				&(((t2_t *)correlation->photons)[i].time));

		result = (result == 2);
	}

	fscanf(stream_in, "\n");

	if ( ! result ) {
		return( feof(stream_in) ? EOF : PC_ERROR_IO );
	} else {
		return(PC_SUCCESS);
	}
}

int t2_correlation_fprintf(FILE *stream_out, correlation_t const *correlation) {
	int i;

	if ( correlation->order == 1 ) {
		t2v_fprintf(stream_out, correlation->photons);
	} else {
		fprintf(stream_out, 
				"%u", 
				((t2_t *)correlation->photons)[0].channel);

		for ( i = 1; i < correlation->order; i++ ) {
			fprintf(stream_out, 
					",%u,%lld",
					((t2_t *)correlation->photons)[i].channel,
					((t2_t *)correlation->photons)[i].time);
		}

		fprintf(stream_out, "\n");
	}

	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
}

int t2_under_max_distance(void const *correlator) {
	long long max = ((correlator_t *)correlator)->max_time_distance;
	t2_t *left = ((correlator_t *)correlator)->left;
	t2_t *right = ((correlator_t *)correlator)->right;

	return( max == 0 || llabs(right->time - left->time) < max );
}

int t2_over_min_distance(void const *correlator) {
	long long min = ((correlator_t *)correlator)->min_time_distance;
	t2_t *left = (t2_t *)((correlator_t *)correlator)->left;
	t2_t *right = (t2_t *)((correlator_t *)correlator)->right;

	return( min == 0 || llabs(right->time - left->time) >= min );
}

void t3_correlate(correlation_t *correlation) {
	int i;

	for ( i = 1; i < correlation->order; i++ ) {
		((t3_t *)(correlation->photons))[i].pulse -=
				((t3_t *)(correlation->photons))[0].pulse;
		((t3_t *)(correlation->photons))[i].time -=
				((t3_t *)(correlation->photons))[0].time; 
	}

	if ( correlation->order != 1 ) {
		((t3_t *)(correlation->photons))[0].pulse = 0;
		((t3_t *)(correlation->photons))[0].time = 0;
	}
}

int t3_correlation_fscanf(FILE *stream_in, correlation_t *correlation) {
	int i;
	int result;

	result = fscanf(stream_in,
			"%u",
			&(((t3_t *)(correlation->photons))[0].channel));

	result = (result == 1);

	((t3_t *)(correlation->photons))[0].pulse = 0;
	((t3_t *)(correlation->photons))[0].time = 0;

	if ( correlation->order == 1 ) {
		result = fscanf(stream_in,
				",%lld,%lld",
				&(((t3_t *)(correlation->photons))[0].pulse),
				&(((t3_t *)(correlation->photons))[0].time));

		result = (result == 2);
	}

	for ( i = 1; result && i < correlation->order; i++ ) {
		result = fscanf(stream_in,
				",%u,%lld,%lld",
				&(((t3_t *)(correlation->photons))[i].channel),
				&(((t3_t *)(correlation->photons))[i].pulse),
				&(((t3_t *)(correlation->photons))[i].time));

		result = (result == 3);
	}

	fscanf(stream_in, "\n");

	if ( ! result ) {
		return( feof(stream_in) ? EOF : PC_ERROR_IO );
	} else {
		return(PC_SUCCESS);
	}
}

int t3_correlation_fprintf(FILE *stream_out, correlation_t const *correlation) {
	int i;

	if ( correlation->order == 1 ) {
		t3v_fprintf(stream_out, correlation->photons);
	} else {
		fprintf(stream_out, 
				"%u",
				((t3_t *)(correlation->photons))[0].channel);

		for ( i = 1; i < correlation->order; i++ ) {
			fprintf(stream_out, 
					",%u,%lld,%lld",
					((t3_t *)correlation->photons)[i].channel,
					((t3_t *)correlation->photons)[i].pulse,
					((t3_t *)correlation->photons)[i].time);
		}

		fprintf(stream_out, "\n");
	}

	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
}

int t3_under_max_distance(void const *correlator) {
	int64_t max_time = ((correlator_t *)correlator)->max_time_distance;
	int64_t max_pulse = ((correlator_t *)correlator)->max_pulse_distance;
	t3_t *left = ((correlator_t *)correlator)->left;
	t3_t *right = ((correlator_t *)correlator)->right;

	return( (max_time == 0 
				|| llabs(right->time - left->time) < max_time)
			&& (max_pulse == 0 
				|| llabs(right->pulse - left->pulse) < max_pulse) );
}

int t3_over_min_distance(void const *correlator) {
	int64_t min_time = ((correlator_t *)correlator)->min_time_distance;
	int64_t min_pulse = ((correlator_t *)correlator)->min_pulse_distance;
	t3_t *left = (t3_t *)((correlator_t *)correlator)->left;
	t3_t *right = (t3_t *)((correlator_t *)correlator)->right;

	return( (min_time == 0 || 
				llabs(right->time - left->time) >= min_time)
			&& (min_pulse == 0 || 
				llabs(right->pulse - left->pulse) >= min_pulse) );
}

