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

#ifndef NUMBER_TO_CHANNELS_H_
#define NUMBER_TO_CHANNELS_H_

#include <stdio.h>
#include "../queue.h"
#include "../photon/photon.h"
#include "../options.h"

typedef struct {
	int flushing;

	long long current_pulse;
	queue_t *queue;
	photon_t photon;

	photon_t previous_photon;

	unsigned int current_channel;
	unsigned int seen_this_pulse;
	int correlate_successive;
} number_to_channels_t;

number_to_channels_t *number_to_channels_alloc(size_t const queue_size);
void number_to_channels_init(number_to_channels_t *number,
		int const correlate_successive);
int number_to_channels_push(number_to_channels_t *number, 
		photon_t const *photon);
int number_to_channels_next(number_to_channels_t *number);
void number_to_channels_flush(number_to_channels_t *number);
void number_to_channels_pulse_over(number_to_channels_t *number);
void number_to_channels_free(number_to_channels_t **number);

int number_to_channels(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
