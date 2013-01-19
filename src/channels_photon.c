#include <stdlib.h>

#include "error.h"
#include "channels.h"
#include "channels_photon.h"
#include "photon.h"

int channels_photon(FILE *stream_in, FILE *stream_out, options_t *options) {
	/* This needs to do two things:
	 * 1. Suppress photons arriving on particular channels.
	 * 2. Retain those photons on the other channels, and apply any time 
	 *    offsets they require. 
	 * 
	 * The first step is easy: we just choose not to add the photon to the
	 * queue of new photons. The second one requires a little more work, 
	 * but can be divided into two steps:
	 * 1. Allocate a fixed array to populate with photons.
	 * 2. For each new photon, add it to the end of the array.
	 * 3. When the array is full, sort it and emit all photons possible.
	 * 4. If the array is full and photons cannot be removed, report that the 
	 *    array (queue) size is not large enough.
	 * 5. At the end of the stream, sort and emit all photons.
	 */
	void *photon;
	photon_queue_t *queue;
	photon_stream_t photons;
	photon_stream_next_t stream_next;
	photon_next_t photon_next;
	photon_window_dimension_t photon_window_dimension;
	void *photon;
	int64_t max_offset_difference = 0;
	size_t photon_size;

	offsets_t *offsets = offsets_alloc(options->channels);

	if ( offsets == NULL ) {
		return(PC_ERROR_MEM);
	} 

	offsets_init(options);

	if ( offsets->offset_time ) {
		max_offset_difference = offset_difference(
				offsets->time_offsets, options->channels);
	}

	queue = photon_queue_alloc(options->queue_size, options->mode);

	if ( queue == NULL ) {
		offsets_free(&offsets);
		return(PC_ERROR_MEM);
	}

	photon_queue_init(queue);

	if ( options->mode == MODE_T2 ) {
		window_dim = t2v_window_dimension;
		photon_size = sizeof(t2_t);
		photon_next = T2V_NEXT(options->binary_in);
		channel_dim = t2v_channel_dimension;
		photon_print = T2V_PRINT(options->binary_out);
	} else if ( options->mode == MODE_T3 ) {
		window_dim = t3v_window_dimension;
		photon_size = sizeof(t3_t);
		photon_next = T3V_NEXT(options->binary_out);
		channel_dim = t3v_channel_dimension;
		photon_print = T3V_PRINT(options->binary_out);
	} else {
		error("Mode not supported: %d\n", options->mode);
		return(PC_ERROR_MODE);
	}

	result = photon_stream_init(&photons, &stream_next, 
			window_dim, photon_next,
			photon_size, stream_in,
			options->set_lower_bound, options->lower_bound,
			options->width,
			options->set_upper_bound, options->upper_bound);

	photon = malloc(photon_size);

	if ( result != PC_SUCCESS || photon == NULL ) {
		error("Could not initialize photon stream.\n");
		offsets_free(&offsets);
		photon_queue_free(&queue);
		return(PC_ERROR_MEM);
	}

	while ( pc_check(stream_next(&photons, photon)) == PC_SUCCESS ) {
		if ( photon_queue_full(queue) ) {
			if ( options->offset_time || options->offset_pulse ) { 
				photon_queue_sort(queue);
			}

			photon_queue_yield_sorted(stream_out, queue, 
					max_offset_difference, photon_print);
		}

		if ( ! (options->suppress_channels &&
				 options->suppressed_channels[channel_dim(photon)]) ) {
			offset_photon(photon, offsets);

			if ( photon_queue_push(queue, photon) ) {
				error("Could not add new photon to queue.\n");
				return(PC_ERROR_UNKNOWN);
			} 
		}
	}

	/* We have reached the end of the new records (whether by error or EOF). 
	 * Process the remaining entries and clean up.
	 */
	debug("Finished processing photons, with a result of %d\n", result);
	if ( ! result ) {
		debug("Yielding the final photons.\n");
		if ( options->offset_time ) {
			photon_queue_sort(queue);
		}
		yield_photon_queue(stream_out, queue, options);
	}
	
	free(photon);
	photon_queue_free(&photons);
	offsets_free(&offsets);

	return(result);
}

void photon_sorted_yield(FILE *stream_out, photon_queue_t *queue, 
		int64_t max_offset_difference, photon_print_t photon_print) {
	photon_t left;
	photon_t right;

	photon_queue_back(queue, &right);
	photon_queue_front(queue, &left);
	while ( ! photon_queue_front(queue, &left) &&
			right.time - left.time > max_offset_difference ) {
		photon_queue_pop(queue, &left);
		print_photon(stream_out, &left);
	}
}

