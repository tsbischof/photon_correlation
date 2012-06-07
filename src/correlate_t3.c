#include <stdio.h>

#include "t3.h"
#include "correlate.h"
#include "correlate_t3.h"
#include "combinations.h"
#include "error.h"
#include "options.h"

t3_queue_t *allocate_t3_queue(options_t *options) {
	int result = 0;
	t3_queue_t *queue;

	queue = (t3_queue_t *)malloc(sizeof(t3_queue_t));
	if ( queue == NULL ) {
		result = -1;
	} else {
		queue->length = options->queue_size;
		queue->left_index = -1;
		queue->right_index = -1;
	
		queue->queue = (t3_t *)malloc(sizeof(t3_t)*queue->length);
		if ( queue->queue == NULL ) {
			result = -1;
		}
	}

	if ( result ) {
		free_t3_queue(&queue);
	}

	return(queue);
}

void free_t3_queue(t3_queue_t **queue) {
	if ( *queue != NULL ) {
		if ( (*queue)->queue != NULL ) {
			free((*queue)->queue);
		}
		free(*queue);
	}
}

t3_t get_queue_item_t3(t3_queue_t *queue, int index) {
	int true_index = (queue->left_index + index) % queue->length;
	return(queue->queue[true_index]);
}

int correlate_t3(FILE *in_stream, FILE *out_stream, options_t *options) {
	/* A t3 record has three parts: 
	 * -channel number
	 * -pulse number
	 * -time relative to the pulse
	 * 
	 * Therefore, we want to output the correlation as relative to the pulse
	 * number and as the t3-like correlation of arrival times. So, do the 
	 * correlation on the channels as integer math, and the correlation on the
	 * times as integer or float math (both, if desired).
	 */
	int result = 0;
	int done = 0;
	t3_queue_t *queue;
	t3_t *correlation_block;
	permutations_t *permutations;
	offsets_t *offsets;

	/* Allocate space in the queue. */
	queue = allocate_t3_queue(options);
	correlation_block = (t3_t *)malloc(sizeof(t3_t)*options->order);
	offsets = allocate_offsets(options->order);
	permutations = make_permutations(options->order, options->positive_only);

	if ( queue == NULL || offsets == NULL || correlation_block == NULL ||
			permutations == NULL ) {
		error("Could not allocate memory for correlation.\n");
		result = -1;
	}

	if ( result ) {
		done = 1;
	}

	/* Start the correlation process. */
	debug("Starting the correlation process.\n");
	while ( ! done && next_t3_queue(in_stream, queue, options) ) {
		/* For each entry in the queue from the left to right index, 
		 * determine the distance by referencing the correct 
		 * channel combination.
		 */	
		correlate_t3_block(out_stream, queue, permutations,
					offsets, correlation_block, options); 
	}

	/* Cleanup */
	free_t3_queue(&queue);
	free_offsets(&offsets);
	free(correlation_block);
	free_permutations(&permutations);
	
	return(result);
}


int next_t3_queue(FILE *in_stream, t3_queue_t *queue, options_t *options) {
	int result;
	long long int starting_index;
	long long int ending_index;

	queue->left_index += 1;
	starting_index = queue->left_index % queue->length;
	ending_index = queue->right_index % queue->length;

	while ( 1 ) {
		if ( feof(in_stream) ) {
			/* If we are at the end of the file, keep moving the left index to
			 * the right, until we reach it.
			 *
			 * Make sure we have options->order many indices to work with.
			 */
			return(queue->left_index < 
					(queue->right_index - options->order + 2));
		} else if ( ending_index > 0 && 
					! valid_distance_t3(&(queue->queue[starting_index]),
								&(queue->queue[ending_index]), options) ) {
			/* Incremented, but still outside the bounds. */
			return(1);
		} else {
			/* Check for a new value. */
			queue->right_index += 1;
			ending_index = queue->right_index % queue->length;
			result = next_t3(in_stream, &(queue->queue[ending_index]));

			if ( result && ! feof(in_stream) ) {
			/* Failed to read a line. We already checked that we are not 
			 * at the end of the stream, therefore we have a read error 
			 * on our hands.
			 */
				error("Error while reading t3 stream.\n");
				return(0);
			} else if ( (queue->right_index - queue->left_index) 
				>= queue->length ) {
				warn("Overflow of queue entries, increase queue size for "
					"accurate results.\n");
			}

			/* By here, we have no error, no EOF, so move along. */
		}
	}
}

int valid_distance_t3(t3_t *left, t3_t *right, options_t *options) {
	return( (options->max_time_distance == 0 
				|| llabs(right->time - left->time) 
						<= options->max_time_distance) 
			&& (options->max_pulse_distance == 0 
				|| llabs(right->pulse_number - left->pulse_number) 
						<= options->max_pulse_distance) );
}

int correlate_t3_block(FILE *out_stream, t3_queue_t *queue, 
		permutations_t *permutations,
		offsets_t *offsets, t3_t *correlation_block, options_t *options) {
	/* For the pairs (left_index, j1, j2,...jn) for j=left_index+1 
 	 * to right_index, determine:
 	 * 1. If the pairs are within the correct distance (leftmost to rightmost)
 	 * 2. What the number of the combination is.
 	 * 3. Based on 2., use the order given by the combination to determine
 	 *    the indices to use to get the ordered correlation.
 	 * 4. Actually print the correlation.
 	 */
	int i;
	int time_distance;
	long long int pulse_distance;
	t3_t left;
	t3_t right;
	int offset;
	int permutation;

	debug("---------------------------------\n");
	debug("Initializing the offset array.\n");
	init_offsets(offsets);
	offsets->limit = queue->right_index - queue->left_index;

	debug("Moving on to the correlation.\n");

	while ( ! next_offsets(offsets) ) {
		/* First, check that the leftmost and rightmost values are acceptable
 		 * for correlation. If not, move on to the next set.
 		 */
		left = get_queue_item_t3(queue, offsets->offsets[0]);
		right = get_queue_item_t3(queue, offsets->offsets[options->order-1]);
		
		if ( valid_distance_t3(&left, &right, options) ) {
			debug("Close enough for correlation.\n");
			for ( permutation = 0; permutation < permutations->n_permutations;
					permutation++  ) {
				offset = offsets->offsets[
						permutations->permutations[permutation][0]];
				left =  get_queue_item_t3(queue, offset);
				fprintf(out_stream, "%d", left.channel);

				for ( i = 1; i < options->order; i++ ) {
					offset = offsets->offsets[
							permutations->permutations[permutation][i]];
					right =	get_queue_item_t3(queue, offset);
					debug("(%u, %lld, %d) <-> (%u, %lld, %d)\n", 
							left.channel, left.pulse_number, left.time,
							right.channel, right.pulse_number, right.time);
	
					time_distance = (right.time - left.time);
					pulse_distance = (right.pulse_number - left.pulse_number);
	
					fprintf(out_stream, 
							",%u,%lld,%d", 
							right.channel, 
							pulse_distance, 
							time_distance);
				}
				fprintf(out_stream, "\n"); 
			}
		} else {
			debug("Not close enough for correlation.\n");
		}
	}
	return(0);
}
