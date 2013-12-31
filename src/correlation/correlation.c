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

#include "correlation.h"
#include "../modes.h"
#include "../photon/t2.h"
#include "../photon/t3.h"
#include "../error.h"

correlation_t *correlation_alloc(int const mode, unsigned int const order) {
	correlation_t *correlation = NULL;

	correlation = (correlation_t *)malloc(sizeof(correlation_t));
	if ( correlation == NULL ) {
		return(correlation);
	}

	correlation->mode = mode;
	correlation->order = order;

	if ( mode == MODE_T2 ) {
		correlation->photon_size = sizeof(t2_t);
	} else if ( mode == MODE_T3 ) {
		correlation->photon_size = sizeof(t3_t);
	} else {
		error("Unsupported mode: %d\n", mode);
		correlation_free(&correlation);
	}

	correlation->photons = malloc(correlation->photon_size*order);

	if ( correlation->photons == NULL ) {
		correlation_free(&correlation);
		return(correlation);
	}

	return(correlation);
}

void correlation_init(correlation_t *correlation) {
	memset(correlation->photons, 
			0, 
			correlation->photon_size*correlation->order);
}

int correlation_set_index(correlation_t *correlation,
		unsigned int const index, void const *photon) {
	if ( index >= correlation->order ) {
		error("Index too large for correlation: %u (limit %u)\n", 
				index,
				correlation->order);
		return(PC_ERROR_INDEX);
	} 

	memcpy(&(((char *)correlation->photons)[index*correlation->photon_size]),
			photon,
			correlation->photon_size);

	return(PC_SUCCESS);
}

void correlation_free(correlation_t **correlation) {
	if ( *correlation != NULL ) {
		free((*correlation)->photons);
		free(*correlation);
	}
}

