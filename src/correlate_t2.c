#include <stdio.h>
#include <stdlib.h>

#include "t2.h"
#include "error.h"
#include "correlate.h"
#include "correlate_t2.h"
#include "combinations.h"

t2_queue_t *allocate_t2_queue(options_t *options) {
	int result = 0;
	t2_queue_t *queue;

	queue = (t2_queue_t *)malloc(sizeof(t2_queue_t));
	if ( queue == NULL ) {
		result = -1;
	} else {
		queue->length = options->queue_size;
		queue->left_index = -1;
		queue->right_index = -1;
	
		queue->queue = (t2_t *)malloc(sizeof(t2_t)*queue->length);
		if ( queue->queue == NULL ) {
			result = -1;
		}
	}

	if ( result ) {
		free_t2_queue(&queue);
	}

	return(queue);
}

void free_t2_queue(t2_queue_t **queue) {
	if ( *queue != NULL ) {
		if ( (*queue)->queue != NULL ) {
			free((*queue)->queue);
		}
		free(*queue);
	}
}

t2_t get_queue_item_t2(t2_queue_t *queue, int index) {
	int true_index = (queue->left_index + index) % queue->length;
	return(queue->queue[true_index]);
}

int correlate_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	/* A t2 correlation is a direct correlation of absolute times. Therefore,
	 * we want to take the difference between times on the channels and output
	 * the channels (in order, so we can later track sign) and the time 
	 * difference.
	 */
	int result = 0;
	int done = 0;
	t2_queue_t *queue;
	t2_t *correlation_block;
	permutations_t *permutations;
	offsets_t *offsets;

	/* Allocate space in the queue. */
	queue = allocate_t2_queue(options);
	correlation_block = (t2_t *)malloc(sizeof(t2_t)*options->order);
	offsets = allocate_offsets(options->order);
	permutations = make_permutations(options->order, options->positive_only);

	if ( queue == NULL || offsets == NULL || correlation_block == NULL ||
				permutations == NULL ) {
		error("Could not allocate memory for correlation..\n");
		result = -1;
	}

	if ( result ) {
		done = 1;
	}

	/* Start the correlation process. */
	debug("Starting the correlation process.\n");
	while ( ! done && next_t2_queue(in_stream, queue, options) ) {
		/* For each entry in the queue from the left to right index, 
		 * determine the distance and sign by referencing the correct 
		 * channel combination.
		 */	
		correlate_t2_block(out_stream, queue, permutations,
					offsets, correlation_block, options); 
	}

	debug("Finished correlation, freeing memory.\n");
	/* Cleanup */
	debug("Freeing t2 queue.\n");
	free_t2_queue(&queue);
	debug("Freeing offsets.\n");
	free_offsets(&offsets);
	debug("Freeing correlation block.\n");
	free(correlation_block);
	debug("Freeing permutations.\n");
	free_permutations(&permutations);
	debug("Done cleaning up.\n");
	
	return(result);
}


int next_t2_queue(FILE *in_stream, t2_queue_t *queue, options_t *options) {
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
					! valid_distance_t2(&(queue->queue[starting_index]),
						&(queue->queue[ending_index]), options) ) {
			/* Incremented, but still outside the distance bounds. */
			return(1);
		} else {
			/* If we still have more file to go, and are not outside the 
 			 * distance bounds, get another entry.
			 */
			queue->right_index += 1;
			ending_index = queue->right_index % queue->length;
			result = fscanf(in_stream, "%d,%llu\n", 
					&(queue->queue[ending_index]).channel,
					&(queue->queue[ending_index]).time);

			if ( result != 2 && ! feof(in_stream) ) {
			/* Failed to read a line. We already checked that we are not 
			 * at the end of the stream, therefore we have a read error 
			 * on our hands.
			 */
				error("Error while reading t2 stream.\n");
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

int valid_distance_t2(t2_t *left, t2_t *right, options_t *options) {
	return( options->max_time_distance == 0 || 
			llabs(right->time - left->time) <= options->max_time_distance);
}

int correlate_t2_block(FILE *out_stream, t2_queue_t *queue, 
		permutations_t *permutations,
		offsets_t *offsets, t2_t *correlation_block, options_t *options) {
	/* For the pairs (left_index, j1, j2,...jn) for j=left_index+1 
 	 * to right_index, determine:
 	 * 1. If the pairs are within the correct distance (leftmost to rightmost)
 	 * 2. What the number of the combination is.
 	 * 3. Based on 2., use the order given by the combination to determine
 	 *    the indices to use to get the ordered correlation.
 	 * 4. Actually print the correlation.
 	 */
	int i;
	long long int time_distance;
	t2_t left;
	t2_t right;
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
		left = get_queue_item_t2(queue, offsets->offsets[0]);
		right = get_queue_item_t2(queue, offsets->offsets[options->order-1]);
		
		if ( valid_distance_t2(&left, &right, options) ) {
			debug("Close enough for correlation (between %lld and %lld to "
					"get %lld/%lld).\n",
					left.time, right.time, right.time-left.time,
					options->max_time_distance);
			/* We have picked out a set of photons of size equal to the order
			 * of the correlation, so we now must determine their time offsets
			 * for all possible permutations of their order.
			 */
			for ( permutation = 0; permutation < permutations->n_permutations;
					permutation++ ) {
				/* The first photon is the reference one, so we should keep
 				 * track of it separately as we iterate over the others.
 				 */
				offset = offsets->offsets[
						permutations->permutations[permutation][0]];
				left = get_queue_item_t2(queue, offset);
				fprintf(out_stream, "%d", left.channel);

				for ( i = 1; i < options->order; i++ ) {
					offset = offsets->offsets[
							permutations->permutations[permutation][i]];
					right = get_queue_item_t2(queue, offset);

					debug("(%d, %lld) <-> (%d, %lld)\n", 
							left.channel, left.time,
							right.channel, right.time);

					time_distance = (right.time - left.time);
					fprintf(out_stream, 
							",%d,%lld", 
							right.channel, 
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
