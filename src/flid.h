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
 *   derived from this software without specific prior written permission.
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
#ifndef FLID_H_
#define FLID_H_

#include <stdio.h>
#include "options.h"
#include "histogram/edges.h"
#include "photon/t3.h"
#include "photon/window.h"
#include "limits.h"

typedef struct {
	photon_window_t window;

	unsigned int **counts;
	edges_t *time_axis;
	edges_int_t *intensity_axis;

	unsigned int total_counts;
	unsigned long long total_lifetime;
} flid_t;

int flid(FILE *stream_in, FILE *stream_out, pc_options_t const *options);

flid_t *flid_alloc(limits_t const *time_limits, 
		limits_int_t const *intensity_limits);
void flid_init(flid_t *flid, long long const window_width);
int flid_push(flid_t *flid, t3_t const *photon);
int flid_flush(flid_t *flid);
int flid_update(flid_t *flid, long long const intensity, double const lifetime);
int flid_fprintf(FILE *stream_out, flid_t const *flid);
void flid_free(flid_t **flid);

#endif
