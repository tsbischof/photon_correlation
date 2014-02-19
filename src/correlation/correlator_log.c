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

#include <math.h>
#include <limits.h>

#include "correlator_log.h"
#include "photon.h"
#include "../photon/stream.h"
#include "../error.h"
#include "../modes.h"

correlator_log_t *correlator_log_alloc(size_t const queue_size) {
	correlator_log_t *correlator = NULL;

	correlator = (correlator_log_t *)malloc(sizeof(correlator_log_t));

	if ( correlator == NULL ) {
		return(correlator);
	}

	correlator->photons = queue_alloc(sizeof(t2_t), queue_size);
	correlator->pairs = queue_alloc(sizeof(unsigned int), queue_size);

	correlator->correlation = correlation_alloc(MODE_T2, 2);

	if ( correlator->photons == NULL ||
			correlator->pairs == NULL ||
			correlator->correlation == NULL ) {
		correlator_log_free(&correlator);
		return(correlator);
	}

	correlator->correlation_print = t2_correlation_fprintf;
	correlator->correlate = t2_correlate;

	return(correlator);
}

void correlator_log_init(correlator_log_t *correlator,
		long long const max_time_distance,
		unsigned int seed) {
	queue_init(correlator->photons);
	queue_init(correlator->pairs);

	correlator->flushing = false;
	correlator->max_time_distance = max_time_distance;

	correlation_init(correlator->correlation);

	/* values for a pseudo-random number generator */
	correlator->a = 1103515245;
	correlator->x = 0xdeadbeef;
	correlator->c = 12345;
	correlator->m = LLONG_MAX / 2;
}

void correlator_log_free(correlator_log_t **correlator) {
	if ( *correlator != NULL ) {
		queue_free(&((*correlator)->photons));
		queue_free(&((*correlator)->pairs));

		correlation_free(&((*correlator)->correlation));
		free(*correlator);
	}
}

int correlator_log_push(correlator_log_t *correlator, t2_t *t2) {
	int result = PC_SUCCESS;

	result = queue_push(correlator->photons, t2);

	if ( result != PC_SUCCESS ) {
		error("Could not add photon to queue: %d\n", result);
		return(result);
	} else {
		return(result);
	}
}

int correlator_log_next(correlator_log_t *correlator) {
	unsigned int right;

	while ( 1 ) {
		if ( queue_size(correlator->photons) < 2 ) {
			debug("Queue too small.\n");
			return(EOF);
		} else if ( queue_empty(correlator->pairs) ) {
			debug("Checking for new pairs (%zu photons in queue).\n",
					queue_size(correlator->photons));
			queue_front(correlator->photons, &(correlator->left));
			queue_back(correlator->photons, &(correlator->right));

			if ( correlator->right.time - correlator->left.time >=
					correlator->max_time_distance ) {
				correlator_log_form_pairs(correlator);

				if ( queue_empty(correlator->pairs) ) {
					queue_pop(correlator->photons, &(correlator->left));
				}
			} else {
				return(EOF);
			}
		} else { 
			debug("Yielding from current pairs.\n");
			queue_pop(correlator->pairs, &right);

			queue_index(correlator->photons, &(correlator->left), 0);
			queue_index(correlator->photons, &(correlator->right), right);

			if ( queue_empty(correlator->pairs) ) {
				debug("Popping.\n");
				queue_pop(correlator->photons, &(correlator->left));
			}
		
			if ( correlator->right.time - correlator->left.time 
					< correlator->max_time_distance ) {
				correlation_set_index(correlator->correlation, 
						0, &(correlator->left));
				correlation_set_index(correlator->correlation, 
						1, &(correlator->right));

				correlator->correlate(correlator->correlation);

				return(PC_SUCCESS);
			}
		}
	}
}

void correlator_log_flush(correlator_log_t *correlator) {
	correlator->flushing = true;

/* flush the whole stream to ensure we sample properly. This may
 * not be necessary, but do it until that can be confirmed.
 */
	queue_init(correlator->photons);
	queue_init(correlator->pairs);
}

int correlator_log_form_pairs(correlator_log_t *correlator) {
	/* Sample some number of photon pairs from an exponential distribution. */
	int result = PC_SUCCESS;
	unsigned int max_index = queue_size(correlator->photons);
	long long span;
	unsigned int samples;
	unsigned int index;
	double x;
	double t;
	int i;

	queue_back(correlator->photons, &(correlator->right));
	queue_front(correlator->photons, &(correlator->left));
	span = correlator->right.time - correlator->left.time;

	samples = queue_size(correlator->photons)/10;

	debug("%u samples for %lld span.\n", samples, span);

	for ( i = 0; i < samples; i++ ) {
		correlator->x = (correlator->a*correlator->x + correlator->c) % 
				correlator->m;
		x = (double)correlator->x/(double)LLONG_MAX;
		t = -log(1-x);

		index = (unsigned int)ceil(queue_size(correlator->photons)*(t/span));
		index = 1;

		if ( index <= max_index ) {
			result = queue_push(correlator->pairs, &index);
			if ( result != PC_SUCCESS ) {
				error("Could not push pair to queue: %u\n", index);
				return(result);
			}

			queue_front(correlator->pairs, &index);
		} else {
			warn("Index too large: %u (limit %u)\n", index, max_index);
		}
	}

	return(PC_SUCCESS);
}

int correlate_log(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	correlator_log_t *correlator = NULL;
	photon_stream_t *photons = NULL;

	if ( options->mode != MODE_T2 ) {
		error("Log correlator only valid for t2 mode.\n");
		result = PC_ERROR_OPTIONS;
	}

	if ( options->order != 2 ) {
		error("Log correlator only valid for g2 (g%u given).\n", 
				options->order);
		result = PC_ERROR_OPTIONS;
	}

	if ( options->max_time_distance == 0 ) {
		error("Must specify a maximum time distance.\n");
		result = PC_ERROR_OPTIONS;
	}

	if ( result == PC_SUCCESS ) {
		correlator = correlator_log_alloc(options->queue_size);
		photons = photon_stream_alloc(MODE_T2);

		if ( correlator == NULL || photons == NULL ) {
			error("Could not allocate correlator or photon stream.\n");
			result = PC_ERROR_OPTIONS;
		}
	}

	if ( result == PC_SUCCESS ) {
		correlator_log_init(correlator, options->max_time_distance,
				options->seed);
		photon_stream_init(photons, stream_in);

		while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
			correlator_log_push(correlator, (t2_t *)photons->photon);

			while ( correlator_log_next(correlator) == PC_SUCCESS ) {
				correlator->correlation_print(stream_out, 
						correlator->correlation);
			}
		}

		correlator_log_flush(correlator);
		while ( correlator_log_next(correlator) == PC_SUCCESS ) {
			correlator->correlation_print(stream_out, correlator->correlation);
		}
	}

	correlator_log_free(&correlator);
	photon_stream_free(&photons);

	return(result);
}
