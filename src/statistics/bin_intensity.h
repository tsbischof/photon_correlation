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

#ifndef BIN_INTENSITY_H_
#define BIN_INTENSITY_H_

#include <stdio.h>
#include "../options.h"
#include "../queue.h"
#include "../photon/photon.h"
#include "../photon/window.h"
#include "../limits.h"
#include "../histogram/edges.h"

typedef struct {
	int mode;
	unsigned int order;
	unsigned int channels;

	int flushing;
	
	limits_t window_limits;
	int window_scale;

	int set_start;
	long long start;
	int set_stop;
	long long stop;

	edges_t *edges;
	unsigned long long **counts;

	photon_channel_dimension_t channel_dim;
	photon_window_dimension_t window_dim;

	queue_t *queue;

	size_t photon_size;
	void *current;

	long long maximum_delay;
} bin_intensity_t;

bin_intensity_t *bin_intensity_alloc(int const mode, unsigned int const order,
		unsigned int const channels, 
		limits_t const *time_limits, int const time_scale,
		limits_t const *pulse_limits, int const pulse_scale,
		size_t const queue_size);
void bin_intensity_init(bin_intensity_t *bin_intensity,
		int set_start, long long start,
		int set_stop, long long stop);
void bin_intensity_free(bin_intensity_t **bin_intensity);
void bin_intensity_increment(bin_intensity_t *bin_intensity);
void bin_intensity_flush(bin_intensity_t *bin_intensity);
int bin_intensity_valid_distance(bin_intensity_t *bin_intensity);
int bin_intensity_push(bin_intensity_t *bin_intensity, void const *photon);

int bin_intensity_fprintf(FILE *stream_out, 
		bin_intensity_t const *bin_intensity);
int bin_intensity_fprintf_bins(FILE *stream_out, 
		bin_intensity_t const *bin_intensity, unsigned int const blanks);
int bin_intensity_fprintf_counts(FILE *stream_out, 
		bin_intensity_t const *bin_intensity);

int bin_intensity(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
