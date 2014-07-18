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

#include <stdio.h>
#include <stdlib.h>

#include "correlator.h"
#include "correlation.h"
#include "photon.h"
#include "../options.h"
#include "../error.h"
#include "../modes.h"
#include "../photon/t2.h"
#include "../photon/t3.h"
#include "../photon/queue.h"

correlator_t *correlator_alloc(int const mode, unsigned int const order,
		size_t const queue_size, int const positive_only,
		long long const min_time_distance, long long const max_time_distance,
		long long const min_pulse_distance, 
		long long const max_pulse_distance ) {
	correlator_t *correlator = NULL;

	correlator = (correlator_t *)malloc(sizeof(correlator_t));

	if ( correlator == NULL ) {
		return(correlator);
	}

	correlator->mode = mode;
	correlator->order = order;

	if ( correlator->mode == MODE_T2 ) {
		correlator->correlate = t2_correlate;
		correlator->correlation_print = t2_correlation_fprintf;
		correlator->under_max_distance = t2_under_max_distance;
		correlator->over_min_distance = t2_over_min_distance;
	} else if ( correlator->mode == MODE_T3 ) {
		correlator->correlate = t3_correlate;
		correlator->correlation_print = t3_correlation_fprintf;
		correlator->under_max_distance = t3_under_max_distance;
		correlator->over_min_distance = t3_over_min_distance;
	} else {
		error("Unsupported mode for correlation: %d\n", correlator->mode);
		correlator_free(&correlator);
		return(correlator);
	}

	correlator->queue = photon_queue_alloc(correlator->mode, queue_size);
	correlator->index_offsets = index_offsets_alloc(correlator->order);
	correlator->permutation = permutation_alloc(correlator->order, 
			positive_only);
	correlator->correlation = correlation_alloc(
			correlator->mode, correlator->order);

	if ( correlator->queue == NULL ||
			correlator->index_offsets == NULL ||
			correlator->permutation == NULL ||
			correlator->correlation == NULL ) {
		correlator_free(&correlator);
		return(correlator);
	}

	correlator->min_time_distance = min_time_distance;
	correlator->max_time_distance = max_time_distance;
	correlator->min_pulse_distance = min_pulse_distance;
	correlator->max_pulse_distance = max_pulse_distance;

	return(correlator);
}

int correlator_init(correlator_t *correlator) {
	queue_init(correlator->queue);
	index_offsets_init(correlator->index_offsets, 0);
	permutation_init(correlator->permutation);
	correlation_init(correlator->correlation);

	correlator->flushing = false;
	correlator->in_block = false;
	correlator->in_permutations = false;
	correlator->yielded = false;

	return(PC_SUCCESS);
}

int correlator_push(correlator_t *correlator, photon_t const *photon) {
	/* For first-order correlation, any photon is considered
	 * valid once it is in the queue. Only let valid photons
	 * through.
	 */
	int status = PC_SUCCESS;

	if ( correlator->order == 1 ) {
		status = queue_push(correlator->queue, photon);

		if ( status != PC_SUCCESS) {
			return(status);
		}

		if ( correlator_valid_distance(correlator) ) {
			return(PC_SUCCESS);
		} else {
			queue_init(correlator->queue);
			return(PC_SUCCESS);
		}
		
	}
	
	return(queue_push(correlator->queue, photon));
}

int correlator_next(correlator_t *correlator) {
	if ( correlator->order == 1 ) {
		while ( ! queue_empty(correlator->queue) ) {
			queue_pop(correlator->queue, correlator->left);
			correlation_set_index(correlator->correlation,
					0,
					correlator->left);

			if ( correlator_valid_distance(correlator) ) {
				return(PC_SUCCESS);
			} 
		} 

		return(EOF);
	}

	while ( true ) {
		if ( correlator->in_block ) {
			debug("In block.\n");
			if ( correlator_next_from_block(correlator) == PC_SUCCESS ) {
				debug("Next from block.\n");
				return(PC_SUCCESS);
			}

			debug("Out of block.\n");
		} else {
			debug("Next block.\n");
			if ( correlator_next_block(correlator) != PC_SUCCESS ) {
				debug("No more blocks.\n");
				return(EOF);
			}
		}
	}

	return(EOF);
}

int correlator_next_block(correlator_t *correlator) {
/* Check that there are enough photons, and that they are far enough apart 
 * to be considered a block (flusihng means they always are)
 * */
	if ( correlator->yielded ) {
		queue_pop(correlator->queue, NULL);
		correlator->yielded = false;
	}

	correlator->in_block = false;
	queue_front(correlator->queue, (void *)&correlator->left);
	queue_back(correlator->queue, (void *)&correlator->right);

	if ( queue_size(correlator->queue) < correlator->order ) {
		debug("Queue underfilled.\n");
		if ( correlator->flushing ) {
			queue_init(correlator->queue);
		}

		return(EOF);
	} else if ( ! correlator->flushing && 
			correlator->under_max_distance(correlator) ) {
		return(EOF);
	} else {
		index_offsets_init(correlator->index_offsets, 
				queue_size(correlator->queue)-1);
		correlator->in_permutations = false;
		correlator->yielded = true;
		correlator->in_block = true;

		return(PC_SUCCESS);
	}
}

int correlator_next_from_block(correlator_t *correlator) {
	while ( true ) {
		if ( correlator->in_permutations ) {
			if ( permutation_next(correlator->permutation) == PC_SUCCESS ) {
				correlator_build_correlation(correlator);
				return(PC_SUCCESS);
			} else {
				correlator->in_permutations = false;
			}
		} else {
			if ( index_offsets_next(correlator->index_offsets) == PC_SUCCESS ) {
				if ( correlator->flushing || 
						correlator_valid_distance(correlator) ) {
					permutation_init(correlator->permutation);
					correlator->in_permutations = true;
				}
			} else {
				correlator->in_block = false;
				return(EOF);
			}
		}
	}
}

void correlator_free(correlator_t **correlator) {
	if ( *correlator != NULL ) {
		queue_free(&((*correlator)->queue));
		index_offsets_free(&((*correlator)->index_offsets));
		permutation_free(&((*correlator)->permutation));
		correlation_free(&((*correlator)->correlation));
		free(*correlator);
		*correlator = NULL;
	}
}

int correlator_valid_distance(correlator_t *correlator) {
	queue_index(correlator->queue,
			(void *)&correlator->left,
			correlator->index_offsets->current_index_offsets->values[0]);
	queue_index(correlator->queue, 
			(void *)&correlator->right,
			correlator->index_offsets->current_index_offsets->values[
				correlator->order-1]);
		
	return( correlator->over_min_distance(correlator) &&
			correlator->under_max_distance(correlator) );
}

int correlator_build_correlation(correlator_t *correlator) {
	int i;
	int result;
	correlation_t *cc = correlator->correlation;
	combination_t *io = correlator->index_offsets->current_index_offsets;
	permutation_t *p = correlator->permutation;

	debug("Populating correlation.\n");
	for ( i = 0; i < correlator->order; i++ ) {
		result = queue_index_copy(correlator->queue,
				&(cc->photons[i]),
				io->values[p->values[i]]);

		if ( result != PC_SUCCESS ) {
			error("Bad index request during correlation: %d\n", 
					io->values[p->values[i]]);
			return(result);
		}
	}

	debug("Correlating photons.\n");
	correlator->correlate(correlator->correlation);

	return(PC_SUCCESS);
}

void correlator_flush(correlator_t *correlator) {
	correlator->flushing = true;
}
