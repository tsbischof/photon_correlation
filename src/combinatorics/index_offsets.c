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

#include <stdio.h>
#include <stdlib.h>

#include "index_offsets.h"
#include "../error.h"

index_offsets_t *index_offsets_alloc(unsigned int const length) {
	index_offsets_t *index_offsets = NULL;

	index_offsets = (index_offsets_t *)malloc(sizeof(index_offsets_t));

	if ( index_offsets == NULL ) {
		return(index_offsets);
	} 

	index_offsets->length = length;
	index_offsets->current_index_offsets = combination_alloc(length, length);

	if ( index_offsets->current_index_offsets == NULL ) {
		index_offsets_free(&index_offsets);
		return(index_offsets);
	} 

	return(index_offsets);
}

void index_offsets_init(index_offsets_t *index_offsets, 
		unsigned int const limit) {
	int i;
	index_offsets->limit = limit;

	combination_init(index_offsets->current_index_offsets);
	index_offsets->yielded = 0;

	for ( i = 0; i < index_offsets->length; i++ ) {
		index_offsets->current_index_offsets->values[i] = i;
	}
}

int index_offsets_next(index_offsets_t *index_offsets) {
	int i;
	unsigned int leading_digit;
	combination_t *current = index_offsets->current_index_offsets;

	if ( index_offsets->yielded ) {
		leading_digit = current->values[1];

		for ( i = current->length - 1; i > 0; i-- ) { 
			current->values[i]++;
			if ( current->values[i] <= index_offsets->limit ) {
				/* No overflow */
				i = 0;
			} else {
				current->values[i] = 0;
			}
		}
	
		if ( current->values[1] == 0 ) {
			current->values[1] = leading_digit+1;
		}
	
		for ( i = 1; i < current->length; i++ ) {
			if ( current->values[i] == 0 ) {
				current->values[i] = current->values[i-1] + 1;
			} else {
				/* nothing, because we already incremented. */
				;
			}

			if ( current->values[i] > index_offsets->limit ) {
				return(PC_COMBINATION_OVERFLOW);
			}
		}

		return(PC_SUCCESS);
	} else {
		index_offsets->yielded = 1;
		return(PC_SUCCESS);
	}
}

void index_offsets_free(index_offsets_t **index_offsets) {
	if ( *index_offsets != NULL ) {
		combination_free(&((*index_offsets)->current_index_offsets));
		free(*index_offsets);
		*index_offsets = NULL;
	}
}
