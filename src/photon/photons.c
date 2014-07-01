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

#include "stream.h"
#include "t2.h"
#include "t3.h"
#include "conversions.h"
#include "../modes.h"
#include "../error.h"

int photons_echo(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	photon_stream_t *photon_stream = photon_stream_alloc(options->mode);

	if ( photon_stream == NULL ) {
		result = PC_ERROR_MEM;
	}

	if ( result == PC_SUCCESS ) {
		photon_stream_init(photon_stream, stream_in);

		while ( photon_stream_next_photon(photon_stream) == PC_SUCCESS) {
			photon_stream->photon_print(stream_out, &photon_stream->photon);
		}
	}

	photon_stream_free(&photon_stream);

	return(result);
}
 
int photons(FILE *stream_in, FILE *stream_out,
		pc_options_t const *options) {
	photon_t photon;
	int result = PC_SUCCESS;
	photon_stream_t *photons;

	photons = photon_stream_alloc(options->mode);

	if ( photons == NULL ) {
		result = PC_ERROR_MEM;
	} 

	if ( result == PC_SUCCESS ) {
		if ( (options->convert == MODE_T2 || options->convert == MODE_T3 ) &&
				options->mode != options->convert &&
				options->repetition_rate == 0 ) {
			error("Must have nonzero repetition rate.\n");
			result = PC_ERROR_ZERO_DIVISION;
		}
	}

	if ( result == PC_SUCCESS ) {
		photon_stream_init(photons, stream_in);
		photon_stream_set_unwindowed(photons);

		if ( options->convert == options->mode ||
				options->convert == MODE_UNKNOWN ) {
			debug("Echo photons.\n");
			while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
				photons->photon_print(stream_out, &(photons->photon));
			}
		} else if ( options->mode == MODE_T2 && options->convert == MODE_T3 ) {
			debug("t2 to t3\n");
			while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
				t2_to_t3(&photons->photon, &photon, options->repetition_rate, 
						options->time_origin);
				t3_fprintf(stream_out, &photon);
			}
		} else if ( options->mode == MODE_T3 && options->convert == MODE_T2 ) {
			debug("t3 to t2\n");
			while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
				t3_to_t2(&photons->photon, &photon, options->repetition_rate, 
						options->time_origin);
				t2_fprintf(stream_out, &photon);
			}
		} else if ( options->mode == MODE_T3 && 
				options->convert == MODE_AS_T2 ) {
			debug("t3 as t2\n");
			while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
				t3_as_t2(&photons->photon, &photon);
				t2_fprintf(stream_out, &photon);
			}
		} else {
			error("Invalid photon conversion: %d to %d\n", 
					options->mode, options->convert);
			result = PC_ERROR_MODE;
		}
	}
		
	photon_stream_free(&photons);

	return(PC_SUCCESS);
}
