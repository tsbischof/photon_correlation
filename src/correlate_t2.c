#include "correlate_t2.h"

#include "error.h"

int correlate_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	/* A t2 correlation is a direct correlation of absolute times. Therefore,
	 * we want to take the difference between times on the channels and output
	 * the channels (in order, so we can later track sign) and the time 
	 * difference.
	 */
	int result = 0;
	long long int record_number = 0;
	int done = 0;
	t2_queue_t *queue;
	t2_t *correlation_block;
	permutations_t *permutations;
	t2_correlation_t *correlation;
	offsets_t *offsets;

	/* Allocate space in the queue. */
	queue = allocate_t2_queue(options->queue_size);
	correlation_block = (t2_t *)malloc(sizeof(t2_t)*options->order);
	offsets = allocate_offsets(options->order);
	permutations = make_permutations(options->order, options->positive_only);
	correlation = allocate_t2_correlation(options);

	if ( queue == NULL || offsets == NULL || correlation_block == NULL ||
				permutations == NULL || correlation == NULL ) {
		error("Could not allocate memory for correlation..\n");
		result = -1;
	}

	done = (result != 0);

	/* Start the correlation process. */
	debug("Starting the correlation process.\n");
	while ( ! done && next_t2_queue_correlate(in_stream, queue, options) ) {
		/* For each entry in the queue from the left to right index, 
		 * determine the distance and sign by referencing the correct 
		 * channel combination.
		 */	
		correlate_t2_block(out_stream, &record_number, queue, permutations,
					offsets, correlation_block, correlation, options); 
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
	debug("Freeing correlation.\n");
	free_t2_correlation(&correlation);
	debug("Done cleaning up.\n");
	
	return(result);
}

int next_t2_queue_correlate(FILE *in_stream, 
		t2_queue_t *queue, options_t *options) {
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
					! under_max_distance_t2(
						&(queue->queue[starting_index]),
						&(queue->queue[ending_index]), options) ) {
			/* Incremented, but still outside the distance bounds. */
			return(1);
		} else {
			/* If we still have more file to go, and are not outside the 
 			 * distance bounds, get another entry.
			 */
			queue->right_index += 1;
			ending_index = queue->right_index % queue->length;

			if ( next_t2(in_stream, &(queue->queue[ending_index]), options)
					&& ! feof(in_stream) ) {

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
	return( under_max_distance_t2(left, right, options) 
			&& over_min_distance_t2(left, right, options) ); 
}

int under_max_distance_t2(t2_t *left, t2_t *right, options_t *options) {
	return( options->max_time_distance == 0 
			|| llabs(right->time - left->time) < options->max_time_distance);
}

int over_min_distance_t2(t2_t *left, t2_t *right, options_t *options) {
	return( llabs(right->time - left->time) >= options->min_time_distance ) ;
}

int correlate_t2_block(FILE *out_stream, long long int *record_number,
		t2_queue_t *queue, 
		permutations_t *permutations,
		offsets_t *offsets, t2_t *correlation_block, 
		t2_correlation_t *correlation, options_t *options) {
	/* For the pairs (left_index, j1, j2,...jn) for j=left_index+1 
 	 * to right_index, determine:
 	 * 1. If the pairs are within the correct distance (leftmost to rightmost)
 	 * 2. What the number of the combination is.
 	 * 3. Based on 2., use the order given by the combination to determine
 	 *    the indices to use to get the ordered correlation.
 	 * 4. Actually print the correlation.
 	 */
	int i;
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
				(*record_number)++;
				print_status("correlate", *record_number, options);

				offset = offsets->offsets[
						permutations->permutations[permutation][0]];
				left = get_queue_item_t2(queue, offset);

				correlation->channels[0] = left.channel;

				for ( i = 1; i < options->order; i++ ) {
					offset = offsets->offsets[
							permutations->permutations[permutation][i]];
					right = get_queue_item_t2(queue, offset);

/*					debug("(%d, %lld) <-> (%d, %lld)\n", 
							left.channel, left.time,
							right.channel, right.time); */
					correlation->channels[i] = right.channel;
					correlation->delays[i] = (right.time - left.time);
				}

				print_t2_correlation(out_stream, correlation, options);
			}
		} else {
			debug("Not close enough for correlation.\n");
		}
	}
	return(0);
}

t2_correlation_t *allocate_t2_correlation(options_t *options) {
	t2_correlation_t *correlation = NULL;
	int result = 0;

	correlation = (t2_correlation_t *)malloc(sizeof(t2_correlation_t));

	if ( correlation == NULL ) {
		result = -1;
	} else {
		correlation->order = options->order;
		correlation->channels = (int *)malloc(
				sizeof(int)*options->order);
		correlation->delays = (t2_delay_t *)malloc(
				sizeof(t2_delay_t)*options->order);

		if ( correlation->channels == NULL || correlation->delays == NULL ) {				result = -1;
		}
	}

	if ( result ) {
		free_t2_correlation(&correlation);
		correlation = NULL;
	}

	return(correlation);
}

void free_t2_correlation(t2_correlation_t **correlation) {
	if ( (*correlation) != NULL ) {
		free((*correlation)->channels);
		free((*correlation)->delays);
	}

	free(*correlation);
}

void print_t2_correlation(FILE *out_stream, t2_correlation_t *correlation,
		options_t *options) {
	int i;

	fprintf(out_stream, "%d,", correlation->channels[0]);

	for ( i = 1; i < correlation->order; i++ ) {	
		fprintf(out_stream, "%d,%lld", correlation->channels[i], 
				correlation->delays[i]);

		if ( i+1 != correlation->order ) {
			fprintf(out_stream, ",");
		}
	}

	fprintf(out_stream, "\n");
}

/* The start-stop mode of correlation assumes that one channel is marked as the
 * starting channel, and another as the stopping channel. Any photon arriving
 * on the start channel replaces and existing photon, and any photon arriving
 * on the stop channel either produces a correlation (if photon on start 
 * channel) or nothing (if no photon)
 */
int correlate_t2_start_stop(FILE *in_stream, FILE *out_stream, 
		options_t *options) {
	t2_t ref_photon;
	t2_t record;
	long long int record_number = 0;

	t2_correlation_t *correlation;

	ref_photon.channel = -1;
	ref_photon.time = -1;

	correlation = allocate_t2_correlation(options);

	debug("Correlating in start-stop mode.\n");

	if ( correlation == NULL ) {
		error("Could not allocate t2 correlation.\n");
		return(-1);
	}

	correlation->channels[0] = 0;
	correlation->channels[1] = 1;

	while ( ! next_t2(in_stream, &record, options) ) {
		if ( record.channel == 0 ) {
			ref_photon.channel = record.channel;
			ref_photon.time = record.time;
		} else if ( record.channel == 1 ) {
			if ( ref_photon.channel == 0 ) {
				/* Correlation! */
				record_number += 1;
				print_status("correlate", record_number, options);

				correlation->delays[1] = record.time - ref_photon.time;

				print_t2_correlation(out_stream, correlation, options);

				ref_photon.channel = -1;
			}
		} else if ( record.channel > 2 ) {
				warn("Start-stop mode only works for 2 channels. "
						"Channel index %d found.\n", record.channel);
		}
	}

	return(0);
}
