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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "values_vector.h"

values_vector_t *values_vector_alloc(unsigned int const length) {
	values_vector_t *vv = (values_vector_t *)malloc(sizeof(values_vector_t));

	if ( vv == NULL ) {
		return(vv);
	}

	vv->length = length;
	vv->values = (long long *)malloc(sizeof(long long)*length);
	
	if ( vv->values == NULL ) {
		values_vector_free(&vv);
		return(vv);
	} 

	return(vv);
}

void values_vector_init(values_vector_t *vv) {
	memset(vv->values, 0, sizeof(long long)*vv->length);
}

void values_vector_free(values_vector_t **vv) {
	if ( *vv != NULL ) {
		free((*vv)->values); 
		free(*vv);
	}
}

long long values_vector_index(values_vector_t const *vv, 
		edges_t ** const edges) {
/* The values vector needs to be transformed into an index by:
 * 1. Extracting the index of each bin based on the edges.
 * 2. Turning those indices into an overall index, as with combinations.
 */
	int i;
	long long result;
	long long base = 1;
	long long index = 0;

	for ( i = vv->length-1; i >= 0; i-- ) {
		result = edges[i]->get_index(edges[i], vv->values[i]);

		if ( result < 0 ) {
			return(result);
		}

		index += base*result;
		base *= edges[i]->n_bins;
	}

	return(index);
}

