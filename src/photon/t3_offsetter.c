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

#include "t3_offsetter.h"
#include "stream.h"
#include "offsets.h"
#include "queue.h"
#include "../error.h"
#include <math.h>

t3_offsetter_t *t3_offsetter_alloc(int const channels,
		size_t const queue_size) {
	t3_offsetter_t *offsetter = NULL;

	offsetter = (t3_offsetter_t *)malloc(sizeof(t3_offsetter_t));

	if ( offsetter == NULL ) {
		return(offsetter);
	}

	offsetter->queue = photon_queue_alloc(MODE_T3, queue_size);
	offsetter->channels = channels;
	offsetter->time_offsets = (long long *)malloc(sizeof(long long)*channels);

	if ( offsetter->queue == NULL || offsetter->time_offsets == NULL ) {
		t3_offsetter_free(&offsetter);
	}

	return(offsetter);
}

void t3_offsetter_init(t3_offsetter_t *offsetter,
		int const offset_time, long long const *time_offsets,
		double const repetition_rate) {
	int i;

	offsetter->flushing = false;

	queue_init(offsetter->queue);
	queue_set_comparator(offsetter->queue, t3_compare);

	offsetter->offset_time = offset_time;

	if ( offset_time ) {
		for ( i = 0; i < offsetter->channels; i++ ) {
			offsetter->time_offsets[i] = time_offsets[i];
		}

		offsetter->offset_span = offset_span(time_offsets, 
				offsetter->channels);

	}

	offsetter->repetition_time = (unsigned long long)fabs(
			floor(1e12/repetition_rate));
}

int t3_offsetter_push(t3_offsetter_t *offsetter, photon_t const *photon) {
	int result = PC_SUCCESS;
	photon_t *current_photon;
	long long new_time;

	result = queue_push(offsetter->queue, photon);

	if ( result == PC_SUCCESS && offsetter->offset_time ) {
		queue_back(offsetter->queue, (void *)&current_photon);

		if ( current_photon->t3.channel < offsetter->channels ) {
			new_time = (offsetter->repetition_time * current_photon->t3.pulse +
					current_photon->t3.time +
					offsetter->time_offsets[current_photon->t3.channel]);

			current_photon->t3.pulse = new_time / offsetter->repetition_time;
			current_photon->t3.time = new_time % offsetter->repetition_time;

			if ( current_photon->t3.time < 0 ) {
				current_photon->t3.pulse -= 1;
				current_photon->t3.time += offsetter->repetition_time;
			}
		} else {
			error("Unexpected channel for offset: %d\n", 
					current_photon->t3.channel);
			result = PC_ERROR_CHANNEL;
		}

		queue_sort(offsetter->queue);
	}

	return(result);
}

int t3_offsetter_next(t3_offsetter_t *offsetter) {
	long long left_time;
	long long right_time;

	if ( offsetter->flushing || ! offsetter->offset_time ) {
		if ( queue_pop(offsetter->queue, (void *)&(offsetter->photon))
				 == PC_SUCCESS ) {
			return(PC_RECORD_AVAILABLE);
		} else {
			return(EOF);
		}
	} else {
		/* Time offsets used, make sure that there is enough time between
		 * photons to ensure that they are certainly sorted.
		 */
		if ( queue_size(offsetter->queue) == 1 ) {
			return(EOF);
		}

		queue_front(offsetter->queue, (void *)&offsetter->left);
		queue_back(offsetter->queue, (void *)&offsetter->right);

		left_time = offsetter->left->t3.pulse*offsetter->repetition_time +
				offsetter->left->t3.time;
		right_time = offsetter->right->t3.pulse*offsetter->repetition_time +
				offsetter->right->t3.time;

		if ( (right_time - left_time) > offsetter->offset_span ) {
			queue_pop(offsetter->queue, (void *)&(offsetter->photon));
			return(PC_RECORD_AVAILABLE);
		} else {
			return(EOF);
		}
	}
}

void t3_offsetter_flush(t3_offsetter_t *offsetter) {
	offsetter->flushing = true;
}

void t3_offsetter_free(t3_offsetter_t **offsetter) {
	if ( *offsetter != NULL ) {
		queue_free(&((*offsetter)->queue));
		free((*offsetter)->time_offsets);
		free(*offsetter);
		*offsetter = NULL;
	}
}

int t3_offsets(FILE *stream_in, FILE *stream_out, pc_options_t const *options) {
	int result = PC_SUCCESS;

	t3_offsetter_t *offsetter;
	photon_stream_t *photons;

	offsetter = t3_offsetter_alloc(options->channels, options->queue_size);
	photons = photon_stream_alloc(MODE_T3);

	if ( offsetter == NULL || photons == NULL ) {
		error("Could not allocate memory for offsetter or photon stream.\n");
		result = PC_ERROR_MEM;
	}

	photon_stream_init(photons, stream_in);
	t3_offsetter_init(offsetter, options->offset_time, options->time_offsets,
			options->repetition_rate);

	while ( result == PC_SUCCESS && 
			photon_stream_next_photon(photons) == PC_SUCCESS ) {
		t3_offsetter_push(offsetter, &(photons->photon));

		while ( t3_offsetter_next(offsetter) == PC_SUCCESS ) {
			t3_fprintf(stream_out, &(offsetter->photon));
		}
	}

	if ( result == PC_SUCCESS ) {
		t3_offsetter_flush(offsetter);
		while ( t3_offsetter_next(offsetter) == PC_SUCCESS ) {
			t3_fprintf(stream_out, &(offsetter->photon));
		}
	}

	t3_offsetter_free(&offsetter);
	photon_stream_free(&photons);

	return(result);
}

