#include <math.h>

#include "error.h"
#include "bin_intensity_t3.h"

int bin_intensity_t3(FILE *stream_in, FILE *stream_out, options_t *options) {
	int result = 0;
	t3_queue_t *queue;
	t3_bin_counts_t *bin_counts;

	t3_t front, back, record;

	int64_t maximum_delay = (int64_t)ceil(options->pulse_limits.upper 
			- options->pulse_limits.lower);

	debug("Maximum delay is %"PRId64"\n", maximum_delay);
	
	queue = allocate_t3_queue(options->queue_size);
	bin_counts = allocate_t3_bin_counts(options);

	if ( queue == NULL || bin_counts == NULL ) {
		result = -1;
	} else {
		while ( !feof(stream_in) ) {
			/* Populate the first photon in the stream. */
			next_t3(stream_in, &record, options);
			if ( options->set_start_time ) {
				if ( record.pulse >= options->start_time ) {
					result = t3_queue_push(queue, &record);
					break;
				} else {
					debug("Photon found which arrives before the start "
							"pulse.\n");
				}
			} else {
				/* Limit not set, photon is valid to process. */
				result = t3_queue_push(queue, &record);
				break;
			}
		}

		if ( result ) {
			error("Error while populating the queue with its first photon.\n");
		}  else {
			/* With the first photon, we can set the limits. */
			if ( options->set_start_time ) {
				front.channel = 0;
				front.pulse = options->start_time;
			} else {
				t3_queue_front(queue, &front);
			}
		
			if ( options->set_stop_time ) {
				back.channel = 0;
				back.pulse = options->stop_time;
			} else {
				t3_queue_back(queue, &back);
			}
		}

		/* Start the real processing. */
		while ( ! result && ! feof(stream_in) ) {
			t3_queue_front(queue, &record);

			debug("Current distance: %"PRId64"\n", record.pulse);
			if ( (back.pulse - record.pulse) > maximum_delay ) {
				debug("Processing a photon.\n");
				t3_bin_counts_increment(bin_counts, &record, &front, &back,
						options);
				t3_queue_pop(queue, &record);
			} else {
				/* Get a new photon. */
				if ( next_t3(stream_in, &record, options) ||
						t3_queue_push(queue, &record) ) {
					if ( ! feof(stream_in) ) {	
						error("Error while reading photon from t3 stream.\n");
						result = -1;
					}
				} else {
					/* Valid photon, and not EOF. */
					if ( ! options->set_stop_time ) { 
						/* Without a specified stop pulse, the last photon seen
						 * is the upper limit.
						 */
						t3_queue_back(queue, &back);
					}
				}
			}
		}
	}

	while ( ! result && t3_queue_size(queue) > 0 ) {
		result = t3_queue_pop(queue, &record);
		t3_bin_counts_increment(bin_counts, &record, &front, &back, options);
	}

	if ( ! result ) {
		print_t3_bin_counts(stream_out, bin_counts, options);
	} else {
		error("Error while processing counts.\n");
	}

	debug("Freeing t3 queue.\n");
	free_t3_queue(&queue);
	debug("Freeing t3 bin counts.\n");
	free_t3_bin_counts(&bin_counts);
	debug("Done freeing memory.\n");
		
	return(result);
}

t3_bin_counts_t *allocate_t3_bin_counts(options_t *options) {
	int result = 0;
	int i,j;
	t3_bin_counts_t *bin_counts = NULL;

	bin_counts = (t3_bin_counts_t *)malloc(sizeof(t3_bin_counts_t));

	if ( bin_counts == NULL ) {
		result = -1;
	} else {
		bin_counts->bins = options->pulse_limits.bins;
		bin_counts->channels = options->channels;

		bin_counts->bin_count = (int64_t **)malloc(
				sizeof(int64_t *)*bin_counts->bins);

		bin_counts->edges = allocate_edges(options->pulse_limits.bins);

		if ( bin_counts->bin_count == NULL ) {
			result = -1;
		} else {
			result = edges_from_limits(bin_counts->edges, 
					&(options->pulse_limits), options->pulse_scale);
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
		free_t3_bin_counts(&bin_counts);
		bin_counts = NULL;
	}

	return(bin_counts);
}

void free_t3_bin_counts(t3_bin_counts_t **bin_counts) {
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

void print_t3_bin_counts(FILE *stream_out, t3_bin_counts_t *bin_counts, 
		options_t *options) {
	int i, j;

	debug("Printing the counts.\n");
	if ( options->binary_out ) {
		for ( i = 0; i < bin_counts->bins; i++ ) {
			fwrite(&(bin_counts->edges->bin_edges[i]),
					1, sizeof(float64_t), stream_out);
			fwrite(&(bin_counts->edges->bin_edges[i+1]),
					1, sizeof(float64_t), stream_out);
			fwrite(bin_counts->bin_count[i], 
					bin_counts->channels, sizeof(int64_t), stream_out);
		}
	} else { 	
		for ( i = 0; i < bin_counts->bins; i++ ) {
			fprintf(stream_out, "%"PRIf64",%"PRIf64",",
					bin_counts->edges->bin_edges[i], 
					bin_counts->edges->bin_edges[i+1]);
			for ( j = 0; j < bin_counts->channels; j++ ) {
				fprintf(stream_out, "%"PRId64, 
						bin_counts->bin_count[i][j]);
				if ( j+1 < bin_counts->channels ) {
					fprintf(stream_out, ",");
				} else {
					fprintf(stream_out, "\n");
				}
			}
		}
	}
}

void t3_bin_counts_increment(t3_bin_counts_t *bin_counts, 
		t3_t *record, t3_t *front, t3_t *back, options_t *options) {
/* This can be done in parallel, since the bins are all distinct. Check to 
 * see whether this actually helps, considering that each increment is very
 * cheap, and the overhead might kill any advantages.
 */
	int i;
	float64_t lower, upper;
	float64_t front_pulse, back_pulse;

	if ( verbose ) {
		debug("Incrementing photon: (");
		print_t3(stderr, record, NO_NEWLINE, options);
		fprintf(stderr, ")\n");
		fprintf(stderr, "       Current front and back: (");
		print_t3(stderr, front, NO_NEWLINE, options);
		fprintf(stderr, ") (");
		print_t3(stderr, back, NO_NEWLINE, options);
		fprintf(stderr, ")\n");
	}

//#pragma omp parallel for private(lower, upper, front_pulse, back_pulse)
	for ( i = 0; i < bin_counts->bins; i++ ) {
		lower = (float64_t)record->pulse + bin_counts->edges->bin_edges[i];
		upper = (float64_t)record->pulse + bin_counts->edges->bin_edges[i+1];
		front_pulse = (float64_t)front->pulse;
		back_pulse = (float64_t)back->pulse;

		if ( (front_pulse <= lower && lower < back_pulse) ||
				(front_pulse <= upper && upper < back_pulse) ) { 
			bin_counts->bin_count[i][record->channel] += 1;
		}
	}
}
