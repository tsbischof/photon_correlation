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

#ifndef THRESHOLD_H_
#define THRESHOLD_H_

#include <stdio.h>
#include "../photon/window.h"
#include "../options.h"
#include "../photon/photon.h"
#include "../photon/queue.h"

typedef struct {
	unsigned int threshold;
	int flushing;
	int yield_window;

	photon_window_dimension_t window_dim;

	photon_window_t window;
	photon_queue_t *queue;
} photon_threshold_t;

photon_threshold_t *photon_threshold_alloc(int const mode, 
		size_t const queue_size);
void photon_threshold_init(photon_threshold_t *pt, 
		long long const window_width, long long const threshold,
		int const set_lower_bound, long long const lower_bound,
		int const set_upper_bound, long long const upper_bound);
int photon_threshold_push(photon_threshold_t *pt, photon_t const *photon);
int photon_threshold_next(photon_threshold_t *pt, photon_t *photon);
void photon_threshold_flush(photon_threshold_t *pt);
void photon_threshold_free(photon_threshold_t **pt);

int photon_threshold(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
