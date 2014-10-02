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

#ifndef MULTI_TAU_H_
#define MULTI_TAU_H_

#include <stdio.h>

#include "../statistics/counts.h"

/* n-channel implementation of the multi-tau algorithm. For performance 
 * purposes, it may end up being worthwhile to write a separate implementation
 * which uses only one channel.
 */
typedef struct {
	unsigned int binning;
	unsigned int registers;
	unsigned int depth;

	unsigned int channels;
	unsigned long long bin_width;

	unsigned long long n_seen;

	double *intensity;

	double ***signal;
	double **accumulated;
	unsigned long long *pushes;
	double **averages;

	double ****g2;
	unsigned long long **tau;
} multi_tau_g2cn_t;

multi_tau_g2cn_t *multi_tau_g2cn_alloc(unsigned int const binning,
		unsigned int const registers, unsigned int const depth,
		unsigned int const channels, unsigned long long const bin_width);
void multi_tau_g2cn_init(multi_tau_g2cn_t *mt);
void multi_tau_g2cn_free(multi_tau_g2cn_t **mt);

void multi_tau_g2cn_push(multi_tau_g2cn_t *mt, counts_t const *counts);

int multi_tau_g2cn_fprintf(FILE *stream_out, multi_tau_g2cn_t const *mt);

#endif
