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

#include <stdlib.h>
#include <string.h>

#include "counts.h"
#include "../error.h"

counts_t *counts_alloc(unsigned int const channels) {
	counts_t *counts = NULL;

	counts = (counts_t *)malloc(sizeof(counts_t));

	if ( counts == NULL ) {
		return(counts);
	}

	counts->channels = channels;
	counts->counts = (unsigned long long *)malloc(
			sizeof(unsigned long long)*counts->channels);

	if ( counts->counts == NULL ) {
		counts_free(&counts);
		return(counts);
	}

	return(counts);
}

void counts_init(counts_t *counts) {
	memset(counts->counts, 0, counts->channels*sizeof(unsigned long long));
	counts->lower = 0;
	counts->upper = 1;
}

int counts_increment(counts_t *counts, unsigned int const index) {
	if ( index < counts->channels ) {
		counts->counts[index]++;
		return(PC_SUCCESS);
	} else {
		return(PC_ERROR_INDEX);
	}
}

int counts_increment_number(counts_t *counts, unsigned int const index, 
		unsigned long long const number) {
	if ( index < counts->channels ) {
		counts->counts[index] += number;
		return(PC_SUCCESS);
	} else {
		return(PC_ERROR_INDEX);
	}
}

void counts_free(counts_t **counts) {
	if ( *counts != NULL ) {
		free((*counts)->counts);
		free(*counts);
		*counts = NULL;
	}
}

int counts_nonzero(counts_t const *counts) {
	int i;

	for ( i = 0; i < counts->channels; i++ ) {
		if ( counts->counts[i] != 0 ) {
			return(true);
		}
	}

	return(false);
}
