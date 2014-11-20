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

#ifndef T3_OFFSETTER_H_
#define T3_OFFSETTER_H_

#include <stdio.h>
#include "../options.h"
#include "../queue.h"
#include "t3.h"
#include "../modes.h"

typedef struct {
	int flushing;

	queue_t *queue;

	int channels;

	int offset_time;
	long long *time_offsets;
	long long offset_span;

	long long repetition_time;

	photon_t *left;
	photon_t *right;

	photon_t photon;
} t3_offsetter_t;

t3_offsetter_t *t3_offsetter_alloc(int const channels, size_t const queue_size);
void t3_offsetter_init(t3_offsetter_t *offsetter, 
		int const offset_time, long long const *time_offsets,
		double const repetition_rate);
int t3_offsetter_push(t3_offsetter_t *offsetter, photon_t const *t3);
int t3_offsetter_next(t3_offsetter_t *offsetter);
void t3_offsetter_flush(t3_offsetter_t *offsetter);
void t3_offsetter_free(t3_offsetter_t **offsetter);

int t3_offsets(FILE *stream_in, FILE *stream_out, pc_options_t const *options);

#endif
