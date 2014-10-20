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

#ifndef INTENSITY_DEPENDENT_GN_H_
#define INTENSITY_DEPENDENT_GN_H_

#include <stdio.h>
#include "options.h"
#include "photon/photon.h"
#include "limits.h"
#include "histogram/edges.h"
#include "correlation/photon_gn.h"
#include "statistics/counts.h"
#include "queue.h"

typedef struct {
	unsigned int intensity_bins;
	unsigned long long window_width;

	photon_window_t window;
	photon_window_dimension_t window_dim;

	queue_t *photon_queue;

	photon_gn_t **gns;
	counts_t *windows_seen;

	edges_int_t *intensities;
} idgn_t;

int intensity_dependent_gn(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

idgn_t *idgn_alloc(int const mode, int const order, int const channels,
		size_t const queue_size, 
		limits_t const *time_limits, limits_t const *pulse_limits,
		limits_int_t const *intensity_limits);
void idgn_init(idgn_t *idgn, long long const window_width);
int idgn_push(idgn_t *idgn, photon_t const *photon);
int idgn_update(idgn_t *idgn);
int idgn_flush(idgn_t *idgn);
int idgn_fprintf(FILE *stream_out, idgn_t const *idgn);
void idgn_free(idgn_t **idgn);

#endif
