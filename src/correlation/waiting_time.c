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

#include "waiting_time.h"
#include <string.h>
#include "error.h"
#include "photon.h"
#include "../photon/stream.h"
#include "../modes.h"

int waiting_time(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	photon_stream_t *photons = NULL;
	waiting_time_t *wt = NULL;

	photons = photon_stream_alloc(options->mode);
	wt = waiting_time_alloc(options->mode);

	if ( photons == NULL || wt == NULL ) {
		error("Could not allocate memory.\n");
		result = PC_ERROR_MEM;
	}

	if ( result == PC_SUCCESS ) {
		photon_stream_init(photons, stream_in);
		photon_stream_set_unwindowed(photons);

		waiting_time_init(wt);

		while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
			waiting_time_push(wt, &(photons->photon));

			while ( waiting_time_next(wt) == PC_SUCCESS ) {
				waiting_time_fprintf(stream_out, wt);
			}
		}

		while ( waiting_time_next(wt) == PC_SUCCESS ) {
			waiting_time_fprintf(stream_out, wt);
		}
	}

	photon_stream_free(&photons);
	waiting_time_free(&wt);

	return(result);
}

waiting_time_t *waiting_time_alloc(int const mode) {
	waiting_time_t *wt = NULL;

	wt = (waiting_time_t *)malloc(sizeof(waiting_time_t));

	if ( wt != NULL ) { 
		wt->correlation = correlation_alloc(mode, 2);

		if ( wt->correlation == NULL ) {
			waiting_time_free(&wt);
		}

		if ( mode == MODE_T2 ) {
			wt->correlate = t2_correlate;
			wt->print = t2_correlation_fprintf;
		} else if ( mode == MODE_T3 ) {
			wt->correlate = t3_correlate;
			wt->print = t3_correlation_fprintf;
		} else {
			waiting_time_free(&wt);
		}
	}

	return(wt);
}

void waiting_time_init(waiting_time_t *wt) {
	wt->n_seen = 0;
	correlation_init(wt->correlation);
}

void waiting_time_push(waiting_time_t *wt, photon_t const *photon) {
	memcpy(&(wt->src), &(wt->dst), sizeof(photon_t));
	memcpy(&(wt->dst), photon, sizeof(photon_t));

	wt->n_seen++;
}

int waiting_time_next(waiting_time_t *wt) {
	if ( wt->n_seen < 2 ) {
		return(EOF);
	} else {
		correlation_set_index(wt->correlation, 0, &(wt->src));
		correlation_set_index(wt->correlation, 1, &(wt->dst));
		wt->correlate(wt->correlation);

		wt->n_seen = 1;

		return(PC_SUCCESS);
	}
}

int waiting_time_fprintf(FILE *stream_out, waiting_time_t *wt) {
	return(wt->print(stream_out, wt->correlation));
}

void waiting_time_free(waiting_time_t **wt) {
	if ( *wt != NULL ) {
		correlation_free(&((*wt)->correlation));
		free(*wt);
		*wt = NULL;
	}
}

