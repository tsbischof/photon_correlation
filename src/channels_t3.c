#include <stdlib.h>

#include "channels.h"
#include "channels_t3.h"

#include "error.h"
#include "t3.h"

int channels_t3(FILE *stream_in, FILE *stream_out, options_t *options) {
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
	t3_t record;
	t3_queue_t *queue;
	int64_t max_offset_difference = 0;

	if ( options->offset_pulse ) {
		debug("Calculating maximum difference between offsets\n");
		max_offset_difference = offset_difference(
				options->pulse_offsets, options->channels);
		debug("Maximum difference: %"PRId64"\n", max_offset_difference);
	}

	debug("Allocating photon queue.\n");
	queue = allocate_t3_queue(options->queue_size);

	if ( queue == NULL ) {
		error("Could not allocate photon queue.\n");
		result = -1;
	} else {
		while ( ! result && ! next_t3(stream_in, &record, options) ) {
			if ( t3_queue_full(queue) ) {
				/* Queue is full, time to deal with it. */
				debug("Full queue, yielding photons as appropriate.\n");
				if ( options->offset_pulse ) { 
					/* Only sort if offsets are applied. Otherwise, we just
					 * waste some time. 
					 */
					t3_queue_sort(queue);
				}
				yield_sorted_t3(stream_out, queue, max_offset_difference,
						options);
				debug("New queue limits: (%"PRId64", %"PRId64")\n",
						queue->left_index, queue->right_index);
			}

			if ( ! (options->suppress_channels &&
					 options->suppressed_channels[record.channel]) ) {
				offset_t3(&record, options);
				/* Insert the new photon, now that everything is cool. */
				if ( t3_queue_push(queue, &record) ) {
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
		if ( options->offset_pulse ) {
			t3_queue_sort(queue);
		}
		yield_t3_queue(stream_out, queue, options);
	}
	
	debug("Freeing photon queue.\n");
	free_t3_queue(&queue);

	return(result);
}

void offset_t3(t3_t *record, options_t *options) {
	if ( options->offset_time ) {
		record->time += options->time_offsets[record->channel];
	}	
	if ( options->offset_pulse ) {
		record->pulse += options->pulse_offsets[record->channel];
	}
}

void yield_sorted_t3(FILE *stream_out, t3_queue_t *queue, 
		int64_t max_offset_difference, options_t *options) {
	int n_printed = 0;

	t3_t left;
	t3_t right;

	t3_queue_back(queue, &right);
	t3_queue_front(queue, &left);
	while ( ! t3_queue_front(queue, &left) &&
			right.pulse - left.pulse > max_offset_difference ) {
		t3_queue_pop(queue, &left);
		print_t3(stream_out, &left,
				NEWLINE, options);
		n_printed++;
	}

	debug("Yielded %d photons\n", n_printed);
}

