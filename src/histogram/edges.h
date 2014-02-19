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

#ifndef HISTOGRAM_EDGES_H_
#define HISTOGRAM_EDGES_H_

#include "../limits.h"

typedef struct {
	unsigned int n_bins;
	int print_label;
	limits_t limits;
	int scale;
	int (*get_index)(void const *edges, long long const value);
	double *bin_edges;
} edges_t;

edges_t *edges_alloc(size_t const n_bins);
int edges_init(edges_t *edges, limits_t const *limits, int const scale,
		int const print_label);
void edges_free(edges_t **edges);

int edges_index_linear(void const *edges, long long const value);
int edges_index_log(void const *edges, long long const value);
int edges_index_log_zero(void const *edges, long long const value);
int edges_index_bsearch(void const *edges, long long const value);

typedef struct {
	unsigned int length;
	int yielded;
	unsigned int *limits;
	unsigned int *values;
} edge_indices_t;

edge_indices_t *edge_indices_alloc(unsigned int const length);
void edge_indices_init(edge_indices_t *edge_indices, edges_t ** const edges);
void edge_indices_free(edge_indices_t **edge_indices);
int edge_indices_next(edge_indices_t *edge_indices);

#endif
