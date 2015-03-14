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

#ifndef NUMBER_H_
#define NUMBER_H_

#include <stdio.h>
#include "counts.h"
#include "../options.h"
#include "../photon/t3.h"

typedef struct {
	unsigned int max_number;
	unsigned int max_seen;

	int first_seen;
	long long last_pulse;
	unsigned long long current_seen;

	int set_start;
	long long start;
	int set_stop;
	long long stop;

	counts_t *counts;
} photon_number_t;

photon_number_t *photon_number_alloc(unsigned int max_number);
void photon_number_init(photon_number_t *number,
		int const set_start, long long const start,
		int const set_stop, long long const stop);
void photon_number_free(photon_number_t **number);

int photon_number_push(photon_number_t *number, photon_t const *photon);
int photon_number_increment(photon_number_t *number, 
		unsigned int const n_photons, unsigned long long seen);
int photon_number_check_max(photon_number_t *number);
int photon_number_flush(photon_number_t *number);
int photon_number_fprintf(FILE *stream_out, photon_number_t const *number);
int photon_number_fprintf_counts(FILE *stream_out, 
		photon_number_t const *number);

int photon_number(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
