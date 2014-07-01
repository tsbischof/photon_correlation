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

#ifndef INTENSITY_H_
#define INTENSITY_H_

#include <stdio.h>

#include "counts.h"
#include "../photon/photon.h"
#include "../photon/window.h"
#include "../options.h"

typedef struct _intensity_photon_t {
/* common to all calculations */
	counts_t *counts;
	unsigned int channels;

	int (*next)(struct _intensity_photon_t *intensity);

/* for calculating the intensity from photons */
	photon_window_t window;

	int count_all;

	int first_photon_seen;
	int flushing;
	int record_available;
	int yielded;

	int photon_held;
	photon_t photon;

	photon_channel_dimension_t channel_dim;
	photon_window_dimension_t window_dim;

	long long last_window_seen;

/* for reading from file */
	FILE *stream_in;
} intensity_photon_t;

/* common routines */
intensity_photon_t *intensity_photon_alloc(unsigned int const channels, 
		int const mode);
int intensity_photon_push(intensity_photon_t *intensity, 
		photon_t const *photon);
void intensity_photon_counts_init(intensity_photon_t *intensity);
void intensity_photon_flush(intensity_photon_t *intensity);
int intensity_photon_next(intensity_photon_t *intensity);
void intensity_photon_free(intensity_photon_t **intensity);

/* for calculating the intensity from photons */
void intensity_photon_init(intensity_photon_t *intensity,
		int count_all,
		long long bin_width,
		int set_lower_bound, long long lower_bound,
		int set_upper_bound, long long upper_bound);

int intensity_photon_increment(intensity_photon_t *intensity,
		unsigned int const channel);
int intensity_photon_next_from_photon(intensity_photon_t *intensity);

/* for reading from stream */
void intensity_photon_init_stream(intensity_photon_t *intensity,
		FILE *stream_in);
int intensity_photon_next_from_stream(intensity_photon_t *intensity);

//typedef int (*intensity_photon_next_t)(FILE *, intensity_photon_t *);
//typedef int (*intensity_photon_print_t)(FILE *, intensity_photon_t const *);

int intensity_photon_fscanf(FILE *stream_in, intensity_photon_t *intensity);
int intensity_photon_fprintf(FILE *stream_in, 
		intensity_photon_t const *intensity);

int intensity_photon(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
