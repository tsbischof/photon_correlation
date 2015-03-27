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

#ifndef TIME_THRESHOLD_H_
#define TIME_THRESHOLD_H_

#include <stdio.h>
#include "../options.h"
#include "../photon/photon.h"
#include "../photon/stream.h"
#include "number_to_channels.h"

typedef struct {
	int photon_held;
	photon_t first_photon;
	photon_t second_photon;
	unsigned long long time_threshold;
	int correlate_successive;
	photon_stream_t *photons;
	number_to_channels_t *numbers;
} photon_time_threshold_t;

photon_time_threshold_t *photon_time_threshold_alloc(size_t const queue_size);
void photon_time_threshold_init(photon_time_threshold_t *ptt, FILE *stream_in,
		unsigned long long const threshold, int const correlate_successive);
int photon_time_threshold_next(photon_time_threshold_t *ptt, photon_t *photon);
void photon_time_threshold_free(photon_time_threshold_t **ptt);

int photon_time_threshold(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
