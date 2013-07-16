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

	number->queue = queue_alloc(sizeof(t3_t), queue_size);

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

	queue_init(number->queue);
}

int number_to_channels_push(number_to_channels_t *number, t3_t const *t3) {
	int result = PC_SUCCESS;
	t3_t photon;
	int i;

	/* Check that no photon on this channel has been seen in 
	 * the current pulse 
	 */
	for ( i = 0; i < queue_size(number->queue); i++ ) {
		queue_index(number->queue, &photon, i);

		if ( photon.pulse == t3->pulse && photon.channel == t3->channel ) {
			break;
		}
	} 

	if ( i == queue_size(number->queue) ) {
		result = queue_push(number->queue, t3);

		if ( result != PC_SUCCESS ) {
			return(result);
		}
	}

	if ( t3->pulse != number->current_pulse ) {
		number_to_channels_pulse_over(number);
	}

	number->current_pulse = t3->pulse;
		
	return(result);
}

int number_to_channels_next(number_to_channels_t *number) {
	t3_t front;
	t3_t back;

	queue_front(number->queue, &front);
	queue_back(number->queue, &back);

	if ( ( number->flushing && ! queue_empty(number->queue)) ||
			( queue_size(number->queue) > 1 && 
				front.pulse != back.pulse ) ) {
		number->photon.channel = number->current_channel;
		number->photon.pulse = front.pulse;

		if ( number->correlate_successive &&
				 number->previous_photon.pulse == number->photon.pulse ) {
			number->photon.time = front.time - number->previous_photon.time;
		} else {
			number->photon.time = front.time;
		}

		number->current_channel++;

		queue_pop(number->queue, &front);

		memcpy(&(number->previous_photon), &front, sizeof(t3_t));

		return(PC_SUCCESS);
	} else {
		return(EOF);
	}
}

void number_to_channels_pulse_over(number_to_channels_t *number) {
	t3_t front;
	t3_t back;
	int i;

	if ( number->flushing ) {
		number->seen_this_pulse = queue_size(number->queue);
	} else {
		queue_front(number->queue, &front);
		for ( i = queue_size(number->queue) - 1; i >= 0; i-- ) {
			queue_index(number->queue, &back, i);
			if ( front.pulse == back.pulse ) {
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
		queue_free(&((*number)->queue));
		free(*number);
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
			number_to_channels_push(number, (t3_t*)photons->photon);
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
