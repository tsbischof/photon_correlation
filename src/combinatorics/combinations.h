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

#ifndef COMBINATIONS_H_
#define COMBINATIONS_H_

#include <stdio.h>
#include "../options.h"

unsigned int pow_int(unsigned int const base, unsigned int const exponent);
unsigned long long powull(unsigned long long const base, 
		unsigned long long const exponent);
unsigned int n_combinations(unsigned int const n, unsigned int const m);
unsigned int n_permutations(unsigned int const n);
unsigned int factorial(unsigned int const n);

typedef struct {
	unsigned int length;
	unsigned int limit;

	int yielded;
	unsigned int *values;
} combination_t;

combination_t *combination_alloc(unsigned int const length,
		unsigned int const limit);
void combination_init(combination_t *combination);
void combination_free(combination_t **combination);
int combination_next(combination_t *combination);
unsigned int combination_index(combination_t const *combination);
int combination_fprintf(FILE *stream_out, combination_t const *combination);

int combinations_dispatch(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif 
