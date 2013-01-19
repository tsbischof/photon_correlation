#include <stdlib.h>

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
	int result = 0;
	photon_t record;
	photon_queue_t *queue;
	int64_t max_offset_difference = 0;

	if ( options->offset_time ) {
		debug("Calculating maximum difference between offsets\n");
		max_offset_difference = offset_difference(
				options->time_offsets, options->channels);
		debug("Maximum difference: %"PRId64"\n", max_offset_difference);
	}

	debug("Allocating photon queue.\n");
	queue = allocate_photon_queue(options->queue_size);

	if ( queue == NULL ) {
		error("Could not allocate photon queue.\n");
		result = -1;
	} else {
		while ( ! result && ! next_photon(stream_in, &record, options) ) {
			if ( photon_queue_full(queue) ) {
				/* Queue is full, time to deal with it. */
				debug("Full queue, yielding photons as appropriate.\n");
				if ( options->offset_time ) { 
					/* Only sort if offsets are applied. Otherwise, we just
					 * waste some time. 
					 */
					photon_queue_sort(queue);
				}
				yield_sorted_photon(stream_out, queue, max_offset_difference,
						options);
				debug("New queue limits: (%"PRId64", %"PRId64")\n",
						queue->left_index, queue->right_index);
			}

			if ( ! (options->suppress_channels &&
					 options->suppressed_channels[record.channel]) ) {
				offset_photon(&record, options);
				/* Insert the new photon, now that everything is cool. */
				if ( photon_queue_push(queue, &record) ) {
					error("Could not add new photon to queue.\n");
					result = -1;
				} 
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
	
	debug("Freeing photon queue.\n");
	free_photon_queue(&queue);

	return(result);
}

void t3v_offset(photon_t *record, options_t *options) {
	if ( options->offset_time ) {
		((t3_t *)record)->time += 
				options->time_offsets[((t3_t *)record)->channel];
	}	
	
	if ( options->offset_pulse ) {
		((t3_t *)record)->pulse +=
				optoins->pulse_offsets[((t3_t *)record)->channel];
	}
}

void t2v_offset(photon_t *record, options_t *options) {
	if ( options->offset_time ) {
		((t2_t *)record)->time +=
				options->time_offsets[((t2_t *)record)->channel];
	
	} 
}

void photon_sorted_yield(FILE *stream_out, photon_queue_t *queue, 
		int64_t max_offset_difference, options_t *options) {
	int n_printed = 0;

	photon_t left;
	photon_t right;

	photon_queue_back(queue, &right);
	photon_queue_front(queue, &left);
	while ( ! photon_queue_front(queue, &left) &&
			right.time - left.time > max_offset_difference ) {
		photon_queue_pop(queue, &left);
		print_photon(stream_out, &left,
				NEWLINE, options);
		n_printed++;
	}

	debug("Yielded %d photons\n", n_printed);
}

