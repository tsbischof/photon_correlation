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

#ifndef SYNCED_T2_H_
#define SYNCED_T2_H_

#include <stdio.h>
#include "../options.h"
#include "t2.h"
#include "t3.h"
#include "../photon/queue.h"

typedef struct {
	unsigned long long sync_index;

	unsigned int sync_channel;
	unsigned int sync_divider;

	photon_t last_sync;
	photon_t next_sync;

	photon_t photon;

	int flushing;
	int first_sync_seen;

	photon_queue_t *queue;
} synced_t2_t;

int synced_t2_dispatch(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);
int synced_t2(FILE *stream_in, FILE *stream_out, pc_options_t const *options);

synced_t2_t *synced_t2_alloc(size_t const queue_size);
void synced_t2_free(synced_t2_t **synced_t2);
void synced_t2_init(synced_t2_t *synced_t2, 
		unsigned int const sync_channel, unsigned int const sync_divider);
int synced_t2_push(synced_t2_t *synced_t2, photon_t const *photon);
void synced_t2_flush(synced_t2_t *synced_t2);
int synced_t2_next(synced_t2_t *synced_t2);

#endif
