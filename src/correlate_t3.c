#include <stdlib.h>

#include "correlate_t3.h"

#include "error.h"

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
	long long record_number = 0;
	t3_queue_t *queue;
	t3_t *correlation_block;
	t3_correlation_t *correlation;
	permutations_t *permutations;
	offsets_t *offsets;

	/* Allocate space in the queue. */
	queue = allocate_t3_queue(options->queue_size);
	correlation_block = (t3_t *)malloc(sizeof(t3_t)*options->order);
	offsets = allocate_offsets(options->order);
	permutations = make_permutations(options->order, options->positive_only);
	correlation = allocate_t3_correlation(options);

	if ( queue == NULL || offsets == NULL || correlation_block == NULL ||
			permutations == NULL || correlation == NULL ) {
		error("Could not allocate memory for correlation.\n");
		result = -1;
	}

	if ( result ) {
		done = 1;
	}

	/* Start the correlation process. */
	debug("Starting the correlation process.\n");
	while ( ! done && next_t3_queue_correlate(in_stream, queue, options) ) {
		/* For each entry in the queue from the left to right index, 
		 * determine the distance by referencing the correct 
		 * channel combination.
		 */	
		correlate_t3_block(out_stream, &record_number, queue, permutations,
					offsets, correlation_block, correlation, options); 
	}

	/* Cleanup */
	free_t3_queue(&queue);
	free_offsets(&offsets);
	free(correlation_block);
	free_permutations(&permutations);
	free_t3_correlation(&correlation);
	
	return(result);
}


int next_t3_queue_correlate(FILE *in_stream, 
		t3_queue_t *queue, options_t *options) {
	long long starting_index;
	long long ending_index;

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
					! under_max_distance_t3(&(queue->queue[starting_index]),
								&(queue->queue[ending_index]), options) ) {
			/* Incremented, but still outside the bounds. */
			return(1);
		} else {
			/* Check for a new value. */
			queue->right_index += 1;
			ending_index = queue->right_index % queue->length;

			if ( next_t3(in_stream, &(queue->queue[ending_index]), options) 
					&& ! feof(in_stream) ) {
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
	return( under_max_distance_t3(left, right, options)
			&& over_min_distance_t3(left, right, options) );
}

int under_max_distance_t3(t3_t *left, t3_t *right, options_t *options) {
	return( (options->max_time_distance == 0
				|| i64abs(right->time - left->time) 
							<= options->max_time_distance)
			&& (options->max_pulse_distance == 0
				|| i64abs(right->pulse - left->pulse) 
							<= options->max_pulse_distance) );
}

int over_min_distance_t3(t3_t *left, t3_t *right, options_t *options) {
	return( i64abs(right->time - left->time) >= options->min_time_distance 
			&& i64abs(right->pulse - left->pulse)
					 >= options->min_pulse_distance ) ;

}

int correlate_t3_block(FILE *out_stream, long long *record_number,
		t3_queue_t *queue,
		permutations_t *permutations,
		offsets_t *offsets, t3_t *correlation_block, 
		t3_correlation_t *correlation, options_t *options) {
	/* For the pairs (left_index, j1, j2,...jn) for j=left_index+1 
 	 * to right_index, determine:
 	 * 1. If the pairs are within the correct distance (leftmost to rightmost)
 	 * 2. What the number of the combination is.
 	 * 3. Based on 2., use the order given by the combination to determine
 	 *    the indices to use to get the ordered correlation.
 	 * 4. Actually print the correlation.
 	 */
	int i;
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
		get_queue_item_t3(&left, queue, offsets->offsets[0]);
		get_queue_item_t3(&right, queue, offsets->offsets[options->order-1]);
		
		if ( valid_distance_t3(&left, &right, options) ) {
			debug("Close enough for correlation.\n");
			for ( permutation = 0; permutation < permutations->n_permutations;
					permutation++  ) {
				(*record_number)++;
				print_status("correlate", *record_number, options);

				offset = offsets->offsets[
						permutations->permutations[permutation][0]];
				get_queue_item_t3(&left, queue, offset);

				correlation->records[0].channel = left.channel;

				for ( i = 1; i < options->order; i++ ) {
					offset = offsets->offsets[
							permutations->permutations[permutation][i]];
					get_queue_item_t3(&right, queue, offset);

/*					debug("(%u, %lld, %d) <-> (%u, %lld, %d)\n", 
							left.channel, left.pulse, left.time,
							right.channel, right.pulse, right.time);*/
					correlation->records[i].channel = right.channel;
					correlation->records[i].pulse = (right.pulse -
							left.pulse);
					correlation->records[i].time = (right.time - left.time);
				}

				print_t3_correlation(out_stream, correlation, NEWLINE, options);
			}
		} else {
			debug("Not close enough for correlation.\n");
		}
	}
	return(0);
}

t3_correlation_t *allocate_t3_correlation(options_t *options) {
	t3_correlation_t *correlation = NULL;
	int result = 0;

	correlation = (t3_correlation_t *)malloc(sizeof(t3_correlation_t));

	if ( correlation == NULL ) {
		result = -1;
	} else {
		correlation->order = options->order;
		correlation->records = (t3_t *)malloc(options->order*sizeof(t3_t));

		if ( correlation->records == NULL ) {
			result = -1;
		}
	}

	if ( result ) {
		free_t3_correlation(&correlation);
		correlation = NULL;
	}

	return(correlation);
}

void free_t3_correlation(t3_correlation_t **correlation) {
	if ( (*correlation) != NULL ) {
		free((*correlation)->records);
	}

	free(*correlation);
}

int next_t3_correlation(FILE *in_stream, t3_correlation_t *correlation,
		options_t *options) {
	int result;
	int i;

	if ( options->binary_in ) {
		result = (fread(&(correlation->records[0].channel),
					sizeof(correlation->records[0].channel),
					1,
					in_stream) != 1);
	} else {
		result = (fscanf(in_stream,
						"%"SCNd32",",
						&(correlation->records[0].channel)) != 1);
	} 

	if ( result && ! feof(in_stream) ) {
		error("Could not read reference channel from stream.\n");
	} else {
		for ( i = 1; ! result && i < options->order; i++ ) {
			result = next_t3(in_stream,
					&(correlation->records[i]),
					options);
			if ( ! result && ! options->binary_in ) {
				fscanf(in_stream, ",");
			}
		}
	}

	if ( result < 0 ) {
		error("Could not read correlated t3 record.\n");
	} 

	return(result);
}

void print_t3_correlation(FILE *out_stream, t3_correlation_t *correlation,
		int print_newline, options_t *options) {
	int i;

	if ( options->binary_out ) {
		fwrite(&(correlation->records[0].channel),
				sizeof(correlation->records[0].channel),
				1,
				out_stream);

		for ( i = 1; i < correlation->order; i++ ) {
			print_t3(out_stream, &(correlation->records[i]), NO_NEWLINE,
					options);
		}
	} else {
		fprintf(out_stream, "%"PRId32",", correlation->records[0].channel);

		for ( i = 1; i < correlation->order; i++ ) {
			print_t3(out_stream, &(correlation->records[i]), NO_NEWLINE,
					options);

			/* All but the last get a comma. */
			if ( i+1 != correlation->order ) {
				fprintf(out_stream, ",");
			}
		}

		if ( print_newline == NEWLINE ) {
			fprintf(out_stream, "\n");
		}
	}
}
