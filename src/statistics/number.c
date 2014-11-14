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

#include "number.h"
#include "../photon/stream.h"
#include "../modes.h"

#include "../error.h"

photon_number_t *photon_number_alloc(unsigned int max_number) {
	photon_number_t *number = NULL;

	number = (photon_number_t *)malloc(sizeof(photon_number_t));

	if ( number == NULL ) {
		return(number);
	} 

	number->max_number = max_number;
	number->counts = counts_alloc(number->max_number+1);

	if ( number->counts == NULL ) {
		photon_number_free(&number);
		return(number);
	}

	return(number);
}

void photon_number_init(photon_number_t *number,
		int const set_start, long long const start,
		int const set_stop, long long const stop) {
	number->first_seen = false;
	number->last_pulse = 0;
	number->current_seen = 0;
	number->max_seen = 0;

	number->set_start = set_start;
	number->start = start;
	number->set_stop = set_stop;
	number->stop = stop;
	
	counts_init(number->counts);
}

void photon_number_free(photon_number_t **number) {
	if ( *number != NULL ) {
		counts_free(&((*number)->counts));
		free(*number);
		*number = NULL;
	}
}

int photon_number_push(photon_number_t *number, photon_t const *photon) {
	int result = PC_SUCCESS;

	/* Verify that the photon falls within the valid range of times. */
	if ( number->set_start && photon->t3.pulse < number->start ) {
		return(PC_SUCCESS);
	} else if ( number->set_stop && number->stop <= photon->t3.pulse ) {
		if ( number->first_seen ) {
			photon_number_increment(number,
				0,
				number->stop - number->last_pulse - 1);

			number->last_pulse = number->stop - 1;
		}

		return(PC_RECORD_AFTER_WINDOW);
	}

	/* If we have found the first photon, it sets the starting point of
	 * time if that has not been set externally. 
	 */
	if ( ! number->first_seen ) {
		number->first_seen = true;
		number->current_seen = 1;

		if ( number->set_start ) {
			result = photon_number_increment(number, 
					0, 
					photon->t3.pulse - number->start);
		}
		
		number->last_pulse = photon->t3.pulse;

		if ( result == PC_SUCCESS ) {
			return(photon_number_check_max(number));
		} else {
			return(result);
		}
	} else {
		if ( photon->t3.pulse == number->last_pulse ) {
			number->current_seen++;
			return(photon_number_check_max(number));
		} else {
			photon_number_increment(number, 
					0, 
					photon->t3.pulse - number->last_pulse - 1);

			result = photon_number_increment(number, number->current_seen, 1);

			number->current_seen = 1;
			number->last_pulse = photon->t3.pulse;

			if ( result == PC_SUCCESS ) {
				return(photon_number_check_max(number));
			} else {
				return(result);
			}
		}
	}
}

int photon_number_increment(photon_number_t *number, 
		unsigned int const n_photons, unsigned long long seen) {
	int result = counts_increment_number(number->counts, n_photons, seen);

	if ( result != PC_SUCCESS ) {
		error("Could not increment for %u photons.\n", n_photons);
	} 

	return(result);
}
	
int photon_number_check_max(photon_number_t *number) {
	if ( number->current_seen > number->max_seen ) {
		number->max_seen = number->current_seen;
	}

	if ( number->max_seen <= number->max_number ) {
		return(PC_SUCCESS);
	} else {
		error("Too many photons: %u\n", number->max_seen);
		return(PC_ERROR_INDEX);
	}
}

int photon_number_flush(photon_number_t *number) {
	int result = PC_SUCCESS;

	if ( number->first_seen ) {
		result = photon_number_increment(number, number->current_seen, 1);

		if ( number->set_stop ) {
			result = photon_number_increment(number, 
					0,
				number->stop - number->last_pulse - 1);
		} 
	}

	return(result);
}

int photon_number_fprintf(FILE *stream_out, photon_number_t const *number) {
	int i;

	for ( i = 0 ; i < number->max_seen+1; i++ ) {
		if ( number->counts->counts[i] != 0 ) {
			fprintf(stream_out, "%d,%llu\n", i, number->counts->counts[i]);
		}
	}

	return( ferror(stream_out) ? PC_ERROR_IO : PC_SUCCESS );
}

int photon_number_fprintf_counts(FILE *stream_out, 
		photon_number_t const *number) {
	int i;

	for ( i = 0 ; i < number->max_seen+1; i++ ) {
		fprintf(stream_out, "%llu", number->counts->counts[i]);
		if ( i != number->max_seen ) {
			fprintf(stream_out, ",");
		}
	}

	fprintf(stream_out, "\n");

	return( ferror(stream_out) ? PC_ERROR_IO : PC_SUCCESS );
}

int photon_number(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) { 
	int result = PC_SUCCESS;
	photon_stream_t *photons;
	photon_number_t *number;

	number = photon_number_alloc(options->channels * 64);
	photons = photon_stream_alloc(MODE_T3);

	if ( number == NULL || photons == NULL ) {
		error("Could not allocate photon stream or numbers.\n");
		result = PC_ERROR_MEM;
	}

	photon_number_init(number,
			options->set_start, options->start,
			options->set_stop, options->stop);
	photon_stream_init(photons, stream_in);
	photon_stream_set_unwindowed(photons);

	debug("Max photons per pulse: %u\n", number->max_number);

	if ( result == PC_SUCCESS) {
		while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
			photon_number_push(number, &(photons->photon));
		}

		photon_number_flush(number);
		photon_number_fprintf(stream_out, number);
	}

	photon_stream_free(&photons);
	photon_number_free(&number);

	return(result);
}
