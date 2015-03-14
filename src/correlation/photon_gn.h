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

#ifndef PHOTON_GN_H_
#define PHOTON_GN_H_

#include <stdio.h>
#include "../options.h"
#include "../correlation/correlator.h"
#include "../histogram/histogram_gn.h"

typedef struct {
	correlator_t *correlator;
	histogram_gn_t *histogram;
} photon_gn_t;

photon_gn_t *photon_gn_alloc(int const mode, int const order, 
		int const channels, size_t const queue_size,
		limits_t const *time_limits, limits_t const *pulse_limits);
void photon_gn_init(photon_gn_t *gn);
int photon_gn_push(photon_gn_t *gn, photon_t const *photon);
int photon_gn_flush(photon_gn_t *gn);
int photon_gn_fprintf(FILE *stream_out, photon_gn_t const *gn);
int photon_gn_fprintf_bins(FILE *stream_out, photon_gn_t const *gn,
			unsigned int const blanks);
int photon_gn_fprintf_counts(FILE *stream_out, photon_gn_t const *gn);
void photon_gn_free(photon_gn_t **gn);

#endif
