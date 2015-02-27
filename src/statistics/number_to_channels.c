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

#include <string.h>

#include "number_to_channels.h"
#include "../modes.h"
#include "../types.h"
#include "../error.h"
#include "../photon/stream.h"

number_to_channels_t *number_to_channels_alloc(size_t const queue_size) {
	number_to_channels_t *number = NULL;

	number = (number_to_channels_t *)malloc(sizeof(number_to_channels_t));

	if ( number == NULL ) {
		return(number);
	}

	number->queue = photon_queue_alloc(MODE_T3, queue_size);

	if ( number->queue == NULL ) {
		number_to_channels_free(&number);
		return(number);
	} 

	return(number);
}

void number_to_channels_init(number_to_channels_t *number,
		int const correlate_successive) {
	number->flushing = false;
	number->current_pulse = 0;
	number->current_channel = 0;
	number->seen_this_pulse = 0;

	number->correlate_successive = correlate_successive;

	photon_queue_init(number->queue);
}

int number_to_channels_push(number_to_channels_t *number,
		photon_t const *photon) {
	int result = PC_SUCCESS;
	photon_t *old = NULL;
	int i;

	/* Check that no photon on this channel has been seen in 
	 * the current pulse 
	 */
	for ( i = 0; i < photon_queue_size(number->queue); i++ ) {
		photon_queue_index(number->queue, &old, i);

		if ( photon->t3.pulse == old->t3.pulse 
				&& photon->t3.channel == old->t3.channel ) {
			break;
	
		}
	} 

	if ( i == photon_queue_size(number->queue) ) {
		result = photon_queue_push(number->queue, photon);

		if ( result != PC_SUCCESS ) {
			return(result);
		}
	}

	if ( photon->t3.pulse != number->current_pulse ) {
		number_to_channels_pulse_over(number);
	}

	number->current_pulse = photon->t3.pulse;

	return(result);
}

int number_to_channels_next(number_to_channels_t *number) {
	photon_t *front = NULL;
	photon_t *back = NULL;

	photon_queue_front(number->queue, &front);
	photon_queue_back(number->queue, &back);

	if ( ( number->flushing && ! photon_queue_empty(number->queue)) ||
			( photon_queue_size(number->queue) > 1 && 
				front->t3.pulse != back->t3.pulse ) ) {
		number->photon.t3.channel = number->current_channel;
		number->photon.t3.pulse = front->t3.pulse;

		if ( number->correlate_successive &&
				 number->previous_photon.t3.pulse == number->photon.t3.pulse ) {
			number->photon.t3.time = front->t3.time - 
				number->previous_photon.t3.time;
		} else {
			number->photon.t3.time = front->t3.time;
		}

		number->current_channel++;
		photon_queue_pop(number->queue, &(number->previous_photon));

		return(PC_SUCCESS);
	} else {
		return(EOF);
	}
}

void number_to_channels_pulse_over(number_to_channels_t *number) {
	photon_t *front = NULL;
	photon_t *back = NULL;
	int i;

	if ( number->flushing ) {
		number->seen_this_pulse = photon_queue_size(number->queue);
	} else {
		photon_queue_front(number->queue, &front);
		for ( i = photon_queue_size(number->queue) - 1; i >= 0; i-- ) {
			photon_queue_index(number->queue, &back, i);
			if ( front->t3.pulse == back->t3.pulse ) {
				break;
			}
		}

		number->seen_this_pulse = i + 1;;
	}

	number->current_channel = (number->seen_this_pulse)*
			(number->seen_this_pulse - 1)/2;
}

void number_to_channels_flush(number_to_channels_t *number) {
	number->flushing = true;
	number_to_channels_pulse_over(number);
}

void number_to_channels_free(number_to_channels_t **number) {
	if ( *number != NULL ) {
		photon_queue_free(&((*number)->queue));
		free(*number);
		*number = NULL;
	}
}

int number_to_channels(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	photon_stream_t *photons;
	number_to_channels_t *number;

	debug("Alloc\n");
	photons = photon_stream_alloc(MODE_T3);
	number = number_to_channels_alloc(options->queue_size);

	if ( photons == NULL || number == NULL ) {
		error("Could not allocate photon stream or number.\n");
		result = PC_ERROR_MEM;
	}

	if ( result == PC_SUCCESS ) {
		debug("Init.\n");
		photon_stream_init(photons, stream_in);
		photon_stream_set_unwindowed(photons);

		number_to_channels_init(number, options->correlate_successive);

		while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
			debug("Found a photon.\n");
			number_to_channels_push(number, &(photons->photon));
			debug("Pushed.\n");

			while ( number_to_channels_next(number) == PC_SUCCESS ) {
				debug("Yielding.\n");
				t3_fprintf(stream_out, &(number->photon));
			}
		}

		debug("Flushing.\n");
		number_to_channels_flush(number);
		while ( number_to_channels_next(number) == PC_SUCCESS ) {
			debug("Yielding.\n");
			t3_fprintf(stream_out, &(number->photon));
		}
	}

	debug("Freeing.\n");
	photon_stream_free(&photons);
	number_to_channels_free(&number);

	return(result);
}
