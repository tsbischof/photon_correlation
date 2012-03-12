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
	combinations_t *combinations;
	combination_t *combination;
	offsets_t *offsets;

	/* Allocate space in the queue. */
	queue = allocate_t2_queue(options);
	correlation_block = (t2_t *)malloc(sizeof(t2_t)*options->order);
	offsets = allocate_offsets(options->channels, options->order);
	combinations = make_combinations(options->channels, options->order);
	combination = allocate_combination(options->channels, options->order);

	if ( queue == NULL || offsets == NULL || correlation_block == NULL ||
			combinations == NULL || combination == NULL ) {
		error("Could not allocate memory for correlation..\n");
		result = -1;
	}

	if ( result ) {
		done = 1;
	}

	/* Start the correlation process. */
	debug("Starting the correlation process.\n");
	while ( ! done && next_t2(in_stream, queue, options) ) {
		/* For each entry in the queue from the left to right index, 
		 * determine the distance and sign by referencing the correct 
		 * channel combination.
		 */	
		correlate_t2_block(out_stream, queue, 
					combinations, combination,
					offsets, correlation_block, options); 
	}

	debug("Finished correlation, freeing memory.\n");
	/* Cleanup */
	debug("Freeing t2 queue.\n");
	free_t2_queue(&queue);
	debug("Freeing combinations.\n");
	free_combinations(&combinations);
	debug("Freeing offsets.\n");
	free_offsets(&offsets);
	debug("Freeing correlation block.\n");
	free(correlation_block);
	debug("Freeing combination.\n");
	free_combination(&combination);
	debug("Done cleaning up.\n");
	
	return(result);
}


int next_t2(FILE *in_stream, t2_queue_t *queue, options_t *options) {
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
				warn("Overflow of queue entries, increase queue size for"
					"accurate results.\n");
			}

			/* By here, we have no error, no EOF, so move along. */
		}
	}
}

int valid_distance_t2(t2_t *left, t2_t *right, options_t *options) {
	return( options->max_time_distance == 0 || 
			abs(right->time - left->time) <= options->max_time_distance);
}

int correlate_t2_block(FILE *out_stream, t2_queue_t *queue, 
		combinations_t *combinations, combination_t *combination,
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
	int combination_index;
	long long int time_distance;
	t2_t left;
	t2_t right;
	int offset;

	debug("---------------------------------\n");
	debug("Initializing the offset array.\n");
	init_offsets(offsets);
	offsets->limit = queue->right_index - queue->left_index;

	debug("Moving on to the correlation.\n");
	if ( verbose ) {
		print_offsets(offsets);
	}

	while ( ! next_offsets(offsets) ) {
		if ( verbose ) {
			debug("######################################\n");
			printf("offsets:\t(");
			for ( i = 0; i < options->order; i++ ) {
				printf("%2d,", offsets->offsets[i]);
			}
			printf("\b)\n");
		}

		/* First, check that the leftmost and rightmost values are acceptable
 		 * for correlation. If not, move on to the next set.
 		 */
		left = get_queue_item_t2(queue, offsets->offsets[0]);
		right = get_queue_item_t2(queue, offsets->offsets[options->order-1]);
		
		if ( valid_distance_t2(&left, &right, options) ) {
			debug("Close enough for correlation.\n");
			/* We have a tuple of offsets from left_index, which means we now 
 			 * want to perform the correlation. First, obtain the index for 
 			 * where we will find this combination in the array of combinations.
	 		 */
			for ( i = 0; i < options->order; i++ ) {
				combination->digits[i] = get_queue_item_t2(queue, 
												offsets->offsets[i]).channel;
				if ( combination->digits[i] >= options->channels ) {
					error("Channel %d exceeds the limit of the run.\n", 
							combination->digits[i]);
					return(-1);
				}
			}

			combination_index = get_combination_index(combination);
			debug("Combination index: %d\n", combination_index);

			/* Now that we have the indices to use, print out the result.
 			 */
			if ( verbose ) {
				fprintf(out_stream, "channels:\t(");
				for ( i = 0; i < options->order; i++ ) {
					fprintf(out_stream, "%2d,", 
							combination->digits[i]);
				}
				fprintf(out_stream, "\b)\nindices:\t(");
				for ( i = 0; i < options->order; i++ ) {
					fprintf(out_stream, "%2d,", 
							combinations->indices[combination_index][i]);
				}
				fprintf(out_stream, "\b)\n"); 
			}

			if ( options->channels_ordered ) {
				offset = offsets->offsets[
							combinations->indices[combination_index][0]];
			} else {
				offset = offsets->offsets[0];
			}
			left = get_queue_item_t2(queue, offset);
			fprintf(out_stream, "%d", left.channel);
			for ( i = 1; i < options->order; i++ ) {
				if ( options->channels_ordered ) {
					offset = offsets->offsets[
									combinations->indices[
										combination_index][i]];
				} else {
					offset = offsets->offsets[i];
				}
				right = get_queue_item_t2(queue, offset);

				debug("(%d, %lld) <-> (%d, %lld)\n", 
						left.channel, left.time,
						right.channel, right.time);

				time_distance = (right.time - left.time);
				fprintf(out_stream, ",%d,%lld", right.channel, time_distance);
			}
			fprintf(out_stream, "\n"); 
		} else {
			debug("Not close enough for correlation.\n");
		}
	}
	return(0);
}
