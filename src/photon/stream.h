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

#ifndef STREAM_H_
#define STREAM_H_

#include <stdio.h>
#include "photon.h"
#include "window.h"

typedef struct _photon_stream_t {
	FILE *stream_in;

	int mode;
	photon_t photon;
	int yielded;

	photon_next_t photon_next;
	photon_print_t photon_print;

	photon_window_dimension_t window_dim;
	photon_channel_dimension_t channel_dim;
	photon_window_t window;

	int (*photon_stream_next)(struct _photon_stream_t *photon_stream);
} photon_stream_t;

photon_stream_t *photon_stream_alloc(int const mode);
void photon_stream_init(photon_stream_t *photon_stream, FILE *stream_in);
void photon_stream_free(photon_stream_t **photons);

void photon_stream_set_unwindowed(photon_stream_t *photons);
void photon_stream_set_windowed(photon_stream_t *photons,
		long long const bin_width,
		int const set_lower_bound, long long const lower_bound,
		int const set_upper_bound, long long const upper_bound);

int photon_stream_next_photon(photon_stream_t *photons);
int photon_stream_next_window(photon_stream_t *photons);

int photon_stream_next_windowed(photon_stream_t *photon_stream);
int photon_stream_next_unwindowed(photon_stream_t *photon_stream);

int photon_stream_eof(photon_stream_t *photons);

#endif
