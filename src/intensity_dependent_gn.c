/*
Copyright (c) 2011-2014, Thomas Bischof
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. Neither the name of the Massachusetts Institute of Technology nor the 
   names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
 */

#include "intensity_dependent_gn.h"
#include "photon/stream.h"
#include "error.h"

int intensity_dependent_gn(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;

	photon_stream_t *photons = NULL;
	idgn_t *idgn = NULL;

	idgn = idgn_alloc();

	if ( idgn == NULL ) {
		result = PC_ERROR_MEM;
	} else {
		photon_stream_init(photons, stream_in);
		photon_stream_set_unwindowed(photons);
		idgn_init(idgn);
	}

	while ( result == PC_SUCCESS && 
			photons->photon_stream_next(photons) == PC_SUCCESS ) {
		result = idgn_push(idgn, &(photons->photon));
	}

	if ( result == PC_SUCCESS ) {
		idgn_flush(idgn);
		idgn_fprintf(stream_out, idgn);
	}
	
	idgn_free(&idgn);

	return(result);
}

idgn_t *idgn_alloc() {
	idgn_t *idgn = NULL;

	return(idgn);
}

void idgn_init(idgn_t *idgn) {

}

int idgn_push(idgn_t *idgn, photon_t const *photon) {
	return(PC_SUCCESS);
}

void idgn_flush(idgn_t *idgn) {

}

int idgn_fprintf(FILE *stream_out, idgn_t const *idgn) {
	return(PC_SUCCESS);
}

void idgn_free(idgn_t **idgn) {
	free(*idgn);
	*idgn = NULL;
}
