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

#ifndef CORRELATOR_H_
#define CORRELATOR_H_

#include "correlation.h"
#include "../photon/stream.h"
#include "../photon/photon.h"
#include "../queue.h"
#include "../combinatorics/permutations.h"
#include "../combinatorics/index_offsets.h"

typedef struct _correlator_t {
	int mode;
	unsigned int order;

	int flushing;
	int in_block;
	int in_permutations;
	int yielded;

	photon_t left;
	photon_t right;

	long long min_pulse_distance;
	long long max_pulse_distance;
	long long min_time_distance;
	long long max_time_distance;

	queue_t *queue;

	index_offsets_t *index_offsets;
	permutation_t *permutation;
	correlation_t *correlation;

	correlate_t correlate;
	correlation_print_t correlation_print;

	int (*under_max_distance)(struct _correlator_t const *correlator);
	int (*over_min_distance)(struct _correlator_t const *correlator);
} correlator_t;

correlator_t *correlator_alloc(int const mode, unsigned int const order,
		size_t const queue_size, int const positive_only,
		long long const min_time_distance, long long const max_time_distance,
		long long const min_pulse_distance, 
		long long const max_pulse_distance);
int correlator_init(correlator_t *correlator);
void correlator_free(correlator_t **correlator);

int correlator_push(correlator_t *correlator, photon_t const *photon);
int correlator_next(correlator_t *correlator);

int correlator_next_block(correlator_t *correlator);
int correlator_next_from_block(correlator_t *correlator);

int correlator_valid_distance(correlator_t *correlator);
int correlator_build_correlation(correlator_t *correlator);
void correlator_flush(correlator_t *correlator);

#endif
