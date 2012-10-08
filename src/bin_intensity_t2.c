#include <omp.h>
#include <math.h>

#include "error.h"
#include "bin_intensity_t2.h"

int bin_intensity_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	int result = 0;
	t2_queue_t *queue;
	t2_bin_counts_t *bin_counts;

	t2_t front, back, record;

	int64_t maximum_delay = (int64_t)ceil(options->time_limits.upper 
			- options->time_limits.lower);

	debug("Maximum delay is %"PRId64"\n", maximum_delay);
	
	queue = allocate_t2_queue(options->queue_size);
	bin_counts = allocate_t2_bin_counts(options);

	if ( queue == NULL || bin_counts == NULL ) {
		result = -1;
	} else {
		while ( !feof(in_stream) ) {
			/* Populate the first photon in the stream. */
			next_t2(in_stream, &record, options);
			if ( options->set_start_time ) {
				if ( record.time >= options->start_time ) {
					result = t2_queue_push(queue, &record);
					break;
				} else {
					debug("Photon found which arrives before the start "
							"time.\n");
				}
			} else {
				/* Limit not set, photon is valid to process. */
				result = t2_queue_push(queue, &record);
				break;
			}
		}

		if ( result ) {
			error("Error while populating the queue with its first photon.\n");
		}  else {
			/* With the first photon, we can set the limits. */
			if ( options->set_start_time ) {
				front.channel = 0;
				front.time = options->start_time;
			} else {
				t2_queue_front(queue, &front);
			}
		
			if ( options->set_stop_time ) {
				back.channel = 0;
				back.time = options->stop_time;
			} else {
				t2_queue_back(queue, &back);
			}
		}

		/* Start the real processing. */
		while ( ! result && ! feof(in_stream) ) {
			t2_queue_front(queue, &record);

			debug("Current distance: %"PRId64"\n", record.time);
			if ( (back.time - record.time) > maximum_delay ) {
				debug("Processing a photon.\n");
				t2_bin_counts_increment(bin_counts, &record, &front, &back,
						options);
				t2_queue_pop(queue, &record);
			} else {
				/* Get a new photon. */
				if ( next_t2(in_stream, &record, options) ||
						t2_queue_push(queue, &record) ) {
					if ( ! feof(in_stream) ) {	
						error("Error while reading photon from t2 stream.\n");
						result = -1;
					}
				} else {
					/* Valid photon, and not EOF. */
					if ( ! options->set_stop_time ) { 
						/* Without a specified stop time, the last photon seen
						 * is the upper limit.
						 */
						t2_queue_back(queue, &back);
					}
				}
			}
		}
	}

	while ( ! result && t2_queue_size(queue) > 0 ) {
		result = t2_queue_pop(queue, &record);
		t2_bin_counts_increment(bin_counts, &record, &front, &back, options);
	}

	if ( ! result ) {
		print_t2_bin_counts(out_stream, bin_counts, options);
	} else {
		error("Error while processing counts.\n");
	}

	debug("Freeing t2 queue.\n");
	free_t2_queue(&queue);
	debug("Freeing t2 bin counts.\n");
	free_t2_bin_counts(&bin_counts);
	debug("Done freeing memory.\n");
		
	return(result);
}

t2_bin_counts_t *allocate_t2_bin_counts(options_t *options) {
	int result = 0;
	int i,j;
	t2_bin_counts_t *bin_counts = NULL;

	bin_counts = (t2_bin_counts_t *)malloc(sizeof(t2_bin_counts_t));

	if ( bin_counts == NULL ) {
		result = -1;
	} else {
		bin_counts->bins = options->time_limits.bins;
		bin_counts->channels = options->channels;

		bin_counts->bin_count = (int64_t **)malloc(
				sizeof(int64_t *)*bin_counts->bins);

		bin_counts->edges = allocate_edges(options->time_limits.bins);

		if ( bin_counts->bin_count == NULL ) {
			result = -1;
		} else {
			result = edges_from_limits(bin_counts->edges, 
					&(options->time_limits), options->time_scale);
		}

		for ( i = 0; ! result && i < bin_counts->bins; i++ ) {
			bin_counts->bin_count[i] = (int64_t *)malloc(
					sizeof(int64_t)*bin_counts->channels);

			if ( bin_counts->bin_count[i] == NULL ) {
				result = -1;
			} else {
				for ( j = 0; j < bin_counts->channels; j++ ) {
					bin_counts->bin_count[i][j] = 0;
				}
			}
		}
	}

	if ( result ) {
		error("Could not allocate memory for the bin counts.\n");
		free_t2_bin_counts(&bin_counts);
		bin_counts = NULL;
	}

	return(bin_counts);
}

void free_t2_bin_counts(t2_bin_counts_t **bin_counts) {
	int i;

	if ( *bin_counts != NULL ) {
		if ( (*bin_counts)->bin_count != NULL ) {
			for ( i = 0; i < (*bin_counts)->bins; i++ ) {
				free((*bin_counts)->bin_count[i]);
			}
			free((*bin_counts)->bin_count);
		}
		free_edges(&((*bin_counts)->edges));
		free(*bin_counts);
	}
}

void print_t2_bin_counts(FILE *out_stream, t2_bin_counts_t *bin_counts, 
		options_t *options) {
	int i, j;

	debug("Printing the counts.\n");
	if ( options->binary_out ) {
		for ( i = 0; i < bin_counts->bins; i++ ) {
			fwrite(&(bin_counts->edges->bin_edges[i]),
					1, sizeof(float64_t), out_stream);
			fwrite(&(bin_counts->edges->bin_edges[i+1]),
					1, sizeof(float64_t), out_stream);
			fwrite(bin_counts->bin_count[i], 
					bin_counts->channels, sizeof(int64_t), out_stream);
		}
	} else { 	
		for ( i = 0; i < bin_counts->bins; i++ ) {
			fprintf(out_stream, "%"PRIf64",%"PRIf64",",
					bin_counts->edges->bin_edges[i], 
					bin_counts->edges->bin_edges[i+1]);
			for ( j = 0; j < bin_counts->channels; j++ ) {
				fprintf(out_stream, "%"PRId64, 
						bin_counts->bin_count[i][j]);
				if ( j+1 < bin_counts->channels ) {
					fprintf(out_stream, ",");
				} else {
					fprintf(out_stream, "\n");
				}
			}
		}
	}
}

void t2_bin_counts_increment(t2_bin_counts_t *bin_counts, 
		t2_t *record, t2_t *front, t2_t *back, options_t *options) {
/* This can be done in parallel, since the bins are all distinct. Check to 
 * see whether this actually helps, considering that each increment is very
 * cheap, and the overhead might kill any advantages.
 */
	int i;
	float64_t lower, upper;
	float64_t front_time, back_time;

	if ( verbose ) {
		debug("Incrementing photon: (");
		print_t2(stderr, record, NO_NEWLINE, options);
		fprintf(stderr, ")\n");
		fprintf(stderr, "       Current front and back: (");
		print_t2(stderr, front, NO_NEWLINE, options);
		fprintf(stderr, ") (");
		print_t2(stderr, back, NO_NEWLINE, options);
		fprintf(stderr, ")\n");
	}

//#pragma omp parallel for private(lower, upper, front_time, back_time)
	for ( i = 0; i < bin_counts->bins; i++ ) {
		lower = (float64_t)record->time + bin_counts->edges->bin_edges[i];
		upper = (float64_t)record->time + bin_counts->edges->bin_edges[i+1];
		front_time = (float64_t)front->time;
		back_time = (float64_t)back->time;

		if ( (front_time <= lower && lower < back_time) ||
				(front_time <= upper && upper < back_time) ) { 
			bin_counts->bin_count[i][record->channel] += 1;
		}
	}
}
