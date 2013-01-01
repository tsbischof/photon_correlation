#include <stdlib.h>

#include "correlate_t2.h"

#include "error.h"

int correlate_t2(FILE *stream_in, FILE *stream_out, options_t *options) {
	/* A t2 correlation is a direct correlation of absolute times. Therefore,
	 * we want to take the difference between times on the channels and output
	 * the channels (in order, so we can later track sign) and the time 
	 * difference.
	 */
	int result = 0;
	int64_t record_number = 0;
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
	while ( ! done && ! next_t2_queue_correlate(stream_in, queue, options) ) {
		/* For each entry in the queue from the left to right index, 
		 * determine the distance and sign by referencing the correct 
		 * channel combination.
		 */	
		correlate_t2_block(stream_out, &record_number, queue, permutations,
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

int next_t2_queue_correlate(FILE *stream_in, 
		t2_queue_t *queue, options_t *options) {
	t2_t left, right;

	debug("Gathering next photons for correlation.\n");

	/* The first entry has been used already, so eliminate it */
	t2_queue_pop(queue, &left);

	while ( 1 ) {
		t2_queue_front(queue, &left);
		t2_queue_back(queue, &right);

		if ( feof(stream_in) ) {
			/* If we are at the end of the file, keep moving the left index to
			 * the right, until we reach it.
			 *
			 * Make sure we have options->order many indices to work with.
			 * 
			 * Return 0 if the queue has enough elements for a correlation,
			 * plus the two photons which bound it. A non-zero result indicates
			 * there are not enough photons for the correlation to be 
			 * performed.
			 */
			return( t2_queue_size(queue) < options->order );
		} else if ( t2_queue_size(queue) > 0 && 
					! under_max_distance_t2(&left, &right, options) ) {
			/* Incremented, but still outside the distance bounds. */
			return(0);
		} else {
			/* If we still have more file to go, and are not outside the 
 			 * distance bounds, get another entry.
			 */
			if ( ! next_t2(stream_in, &right, options) ) {
				/* Found a photon. */
				if ( t2_queue_push(queue, &right) ) {
					error("Could not add next photon to queue.\n");
					return(-1);
				}
			} else if ( ! feof(stream_in) ) {
				/* No photon, not at the end of the stream. */
				error("Error while reading t2 stream.\n");
				return(-1);
			} else {
				/* No photon, but at the end of the stream. The main loop
				 * will take care of this.
				 */
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
			|| i64abs(right->time - left->time) < options->max_time_distance);
}

int over_min_distance_t2(t2_t *left, t2_t *right, options_t *options) {
	return( i64abs(right->time - left->time) >= options->min_time_distance ) ;
}

int correlate_t2_block(FILE *stream_out, int64_t *record_number,
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
	offsets->limit = t2_queue_size(queue) - 1;

	debug("Moving on to the correlation.\n");

	while ( ! next_offsets(offsets) ) {
		/* First, check that the leftmost and rightmost values are acceptable
 		 * for correlation. If not, move on to the next set.
 		 */
		t2_queue_index(queue, &left, offsets->offsets[0]);
		t2_queue_index(queue, &right, offsets->offsets[options->order-1]);
		
		if ( valid_distance_t2(&left, &right, options)  ) {
			if ( verbose ) {
				debug("Close enough for correlation of: (");
				print_t2(stderr, &left, NO_NEWLINE, options);
				fprintf(stderr, ") and (");
				print_t2(stderr, &right, NO_NEWLINE, options);
				fprintf(stderr, ")\n");
			}
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
				t2_queue_index(queue, &left, offset);

				correlation->records[0].channel = left.channel;

				for ( i = 1; i < options->order; i++ ) {
					offset = offsets->offsets[
							permutations->permutations[permutation][i]];
					t2_queue_index(queue, &right, offset);

					debug("(%"PRId32",%"PRId64") <-> "
							"(%"PRId32",%"PRId64")\n", 
							left.channel, left.time,
							right.channel, right.time);
					correlation->records[i].channel = right.channel;
					correlation->records[i].time = (right.time - left.time);
				}

				print_t2_correlation(stream_out, correlation, NEWLINE, options);
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
		correlation->records = (t2_t *)malloc(sizeof(t2_t)*(options->order));

		if ( correlation->records == NULL ) {
			result = -1;
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
		free((*correlation)->records);
	}

	free(*correlation);
}

int next_t2_correlation(FILE *stream_in, t2_correlation_t *correlation,
			options_t *options) {
	int result;
	int i;

	if ( options->binary_in ) {
		result = (fread(&(correlation->records[0].channel), 
					sizeof(correlation->records[0].channel),
					1,
					stream_in) != 1);
	} else {
		result = (fscanf(stream_in, 
						"%"SCNd32",", 
						&(correlation->records[0].channel)) != 1);
	}
	
	if ( result && ! feof(stream_in) ) {
		error("Could not read reference channel from stream\n");
	} else { 
		for ( i = 1; ! result && i < options->order; i++ ) {
			result = next_t2(stream_in, 
					&(correlation->records[i]),
					options);
			if ( ! result && ! options->binary_in ) {
				fscanf(stream_in, ",");
			}
		}
	}

	if ( result < 0 ) {
		error("Could not read correlated t2 record.\n");
	}
	
	return(result);
}

void print_t2_correlation(FILE *stream_out, t2_correlation_t *correlation,
		int print_newline, options_t *options) {
	int i;

	if ( options->binary_out ) {
		fwrite(&(correlation->records[0].channel), 
				sizeof(correlation->records[0].channel),
				1,
				stream_out);

		for ( i = 1; i < correlation->order; i++ ) {
			print_t2(stream_out, &(correlation->records[i]), NO_NEWLINE, 
					options);
		}
	} else {
		fprintf(stream_out, "%"PRId32",", correlation->records[0].channel);
		
		for ( i = 1; i < correlation->order; i++ ) {
			print_t2(stream_out, &(correlation->records[i]), NO_NEWLINE,
					options);
			
			/* All but the last get a comma. */
			if ( i+1 != correlation->order ) {
				fprintf(stream_out, ",");
			}
	
		}

		if ( print_newline == NEWLINE ) {
			fprintf(stream_out, "\n");
		}
	}
}

/* The start-stop mode of correlation assumes that one channel is marked as the
 * starting channel, and another as the stopping channel. Any photon arriving
 * on the start channel replaces and existing photon, and any photon arriving
 * on the stop channel either produces a correlation (if photon on start 
 * channel) or nothing (if no photon)
 */
int correlate_t2_start_stop(FILE *stream_in, FILE *stream_out, 
		options_t *options) {
	t2_t ref_photon;
	t2_t record;
	int64_t record_number = 0;

	t2_correlation_t *correlation;

	ref_photon.channel = -1;
	ref_photon.time = -1;

	correlation = allocate_t2_correlation(options);

	debug("Correlating in start-stop mode.\n");

	if ( correlation == NULL ) {
		error("Could not allocate t2 correlation.\n");
		return(-1);
	}

	correlation->records[0].channel = 0;
	correlation->records[1].channel = 1;

	while ( ! next_t2(stream_in, &record, options) ) {
		if ( record.channel == 0 ) {
			ref_photon.channel = record.channel;
			ref_photon.time = record.time;
		} else if ( record.channel == 1 ) {
			if ( ref_photon.channel == 0 ) {
				/* Correlation! */
				record_number += 1;
				print_status("correlate", record_number, options);

				correlation->records[1].time = record.time - ref_photon.time;

				print_t2_correlation(stream_out, correlation, NEWLINE, options);

				ref_photon.channel = -1;
			}
		} else if ( record.channel > 2 ) {
				warn("Start-stop mode only works for 2 channels. "
						"Channel index %"PRId32" found.\n", record.channel);
		}
	}

	return(0);
}
