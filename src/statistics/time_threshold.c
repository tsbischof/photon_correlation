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

#include "time_threshold.h"
#include "string.h"
#include "../error.h"
#include "../modes.h"

/*
This code is designed to take an incoming stream of t3 photons, filter for
two-photon events, then divide those events into two groups based on whether
the first photon arrived before or after the specifed time.

This implements the following algorithm:
--------------------------------------------------------------------------------
held = None

for photon in filter(lambda p: p.channel in [1, 2], 
                     number_to_channels(photons)):
    if photon.channel == 1:
        held = photon
        if held.time < threshold:
            held.channel = 0
        else:
            held.channel = 2
    else:
        photon.channel = held.channel + 1
        yield(held)
        yield(photon)
 */


photon_time_threshold_t *photon_time_threshold_alloc(size_t const queue_size) {
	photon_time_threshold_t *ptt = NULL;

	ptt = (photon_time_threshold_t *)malloc(sizeof(photon_time_threshold_t));

	if ( ptt == NULL ) {
		return(ptt);
	}

	ptt->photons = photon_stream_alloc(MODE_T3);
	ptt->numbers = number_to_channels_alloc(queue_size);

	if ( ptt->photons == NULL || ptt->numbers == NULL ) {
		photon_time_threshold_free(&ptt);
	}

	return(ptt);
}

void photon_time_threshold_init(photon_time_threshold_t *ptt, FILE *stream_in,
		unsigned long long const threshold, int const correlate_successive) {
	ptt->photon_held = false;
	photon_stream_init(ptt->photons, stream_in);
	number_to_channels_init(ptt->numbers, correlate_successive);
	ptt->time_threshold = threshold;
}

int photon_time_threshold_next(photon_time_threshold_t *ptt, photon_t *photon) {
	if ( ptt->photon_held ) {
		memcpy(photon, &(ptt->second_photon), sizeof(photon_t));
		ptt->photon_held = false;
		return(PC_SUCCESS);
	} else {
		while ( photon_stream_next_photon(ptt->photons) == PC_SUCCESS ) {
			number_to_channels_push(ptt->numbers, &(ptt->photons->photon));

			while ( number_to_channels_next(ptt->numbers) == PC_SUCCESS ) {
				if ( ptt->numbers->photon.t3.channel == 1 ) {
					memcpy(&(ptt->first_photon), &(ptt->numbers->photon),
							sizeof(photon_t));
					ptt->first_photon.t3.channel = (ptt->first_photon.t3.time 
							 < ptt->time_threshold) ? 0 : 2;
				} else if ( ptt->numbers->photon.t3.channel == 2 ) {
					memcpy(&(ptt->second_photon), &(ptt->numbers->photon),
							sizeof(photon_t));
					ptt->photon_held = true;
					ptt->second_photon.t3.channel = ptt->first_photon.t3.channel
							+ 2;

					memcpy(photon, &(ptt->first_photon), sizeof(photon_t));
					return(PC_SUCCESS);
				}
			}
		}
	}

	return(EOF);
}

void photon_time_threshold_free(photon_time_threshold_t **ptt) {
	if ( *ptt != NULL ) {
		photon_stream_free(&((*ptt)->photons));
		number_to_channels_free(&((*ptt)->numbers));
		free(*ptt);
		*ptt = NULL;
	}
}

int photon_time_threshold(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	photon_t photon;
	photon_time_threshold_t *ptt;

	debug("Allocating memory\n");
	ptt = photon_time_threshold_alloc(options->queue_size);

	if ( ptt == NULL ) {
		error("Could not allocate memory.\n");
		result = PC_ERROR_MEM;
	} else {
		photon_time_threshold_init(ptt, stream_in, 
				options->time_threshold, options->correlate_successive);
	}

	if ( result == PC_SUCCESS ) {
		debug("Starting stream\n");
		while ( photon_time_threshold_next(ptt, &photon) == PC_SUCCESS ) {
			t3_fprintf(stream_out, &photon);
		}
	}

	photon_time_threshold_free(&ptt);

	return(result);
}
