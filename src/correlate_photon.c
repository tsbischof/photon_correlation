#include <stdlib.h>
#include <string.h>

#include "correlate_photon.h"
#include "error.h"
#include "modes.h"
#include "t2.h"
#include "t3.h"

int correlate_photon(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	photon_stream_t *photon_stream = photon_stream_alloc(options);
	correlator_t *correlator = correlator_alloc(options);

	debug("Allocating correlator, photon stream.\n");
	if ( correlator == NULL || photon_stream == NULL ) {
		error("Could not allocate correlator (%p), photon stream (%p).\n",
				correlator, photon_stream);
		correlator_free(&correlator);
		photon_stream_free(&photon_stream);
		return(PC_ERROR_MEM);
	}

	debug("Initializing correlator, photon stream.\n");
	photon_stream_init(photon_stream, stream_in, options);
	correlator_init(correlator, photon_stream, options);

	debug("Streaming correlations.\n");
	while ( correlator_next(correlator) == PC_SUCCESS ) {
		correlator->correlation_print(stream_out, 
				correlator->current_correlation);
	} 

	debug("Freeing correlator, photon stream.\n");
	correlator_free(&correlator);
	photon_stream_free(&photon_stream);

	return(PC_SUCCESS);
}

int correlations_echo(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result;
	correlation_next_t next;
	correlation_print_t print;
	correlation_t *correlation;

	if ( options->mode == MODE_T2 ) {
		next = t2_correlation_fscanf;
		print = t2_correlation_fprintf;
	} else if ( options->mode == MODE_T3 ) {
		next = t3_correlation_fscanf;
		print = t3_correlation_fprintf;
	} else {
		error("Unsupported mode: %d\n", options->mode);
		return(PC_ERROR_MODE);
	}

	correlation = correlation_alloc(options->mode, options->order);

	if ( correlation == NULL ) {
		error("Could not allocate correlation.\n");
		return(PC_ERROR_MEM);
	}

	correlation_init(correlation);

	while ( (result = next(stream_in, correlation)) == PC_SUCCESS ) {
		print(stream_out, correlation);
	}

	correlation_free(&correlation);

	return(PC_SUCCESS);
}

correlation_t *correlation_alloc(int const mode, unsigned int const order) {
	correlation_t *correlation = NULL;

	correlation = (correlation_t *)malloc(sizeof(correlation_t));
	if ( correlation == NULL ) {
		return(correlation);
	}

	correlation->mode = mode;
	correlation->order = order;

	if ( mode == MODE_T2 ) {
		correlation->photon_size = sizeof(t2_t);
	} else if ( mode == MODE_T3 ) {
		correlation->photon_size = sizeof(t3_t);
	} else {
		error("Unsupported mode: %d\n", mode);
		correlation_free(&correlation);
	}

	correlation->photons = malloc(correlation->photon_size*order);

	if ( correlation->photons == NULL ) {
		correlation_free(&correlation);
		return(correlation);
	}

	return(correlation);
}

void correlation_init(correlation_t *correlation) {
	memset(correlation->photons, 
			0, 
			correlation->photon_size*correlation->order);
}

int correlation_set_index(correlation_t *correlation,
		void const *photon,
		unsigned int const index) {
	if ( index >= correlation->order ) {
		error("Index too large for correlation: %u (limit %u)\n", 
				index,
				correlation->order);
		return(PC_ERROR_INDEX);
	} 

	memcpy(&(correlation->photons[index*correlation->photon_size]),
			photon,
			correlation->photon_size);

	return(PC_SUCCESS);
}

void correlation_free(correlation_t **correlation) {
	if ( *correlation != NULL ) {
		free((*correlation)->photons);
		free(*correlation);
	}
}

correlator_t *correlator_alloc(pc_options_t const *options) {
	correlator_t *correlator = NULL;

	correlator = (correlator_t *)malloc(sizeof(correlator_t));

	if ( correlator == NULL ) {
		return(correlator);
	}

	correlator->mode = options->mode;
	correlator->order = options->order;

	if ( correlator->mode == MODE_T2 ) {
		correlator->photon_size = sizeof(t2_t);
		correlator->correlate = t2_correlate;
		correlator->correlation_print = t2_correlation_fprintf;
		correlator->under_max_distance = t2_under_max_distance;
		correlator->over_min_distance = t2_over_min_distance;
	} else if ( correlator->mode == MODE_T3 ) {
		correlator->photon_size = sizeof(t3_t);
		correlator->correlate = t3_correlate;
		correlator->correlation_print = t3_correlation_fprintf;
		correlator->under_max_distance = t3_under_max_distance;
		correlator->over_min_distance = t3_over_min_distance;
	} else {
		error("Unsupported mode for correlation: %d\n", correlator->mode);
		correlator_free(&correlator);
		return(correlator);
	}

	correlator->photon_queue = photon_queue_alloc(
			options->queue_size, correlator->mode);
	correlator->index_offsets = index_offsets_alloc(correlator->order);
	correlator->photon_permutation = permutation_alloc(
			correlator->order, options->positive_only);
	correlator->current_correlation = correlation_alloc(
			correlator->mode, correlator->order);

	correlator->left = (void *)malloc(correlator->photon_size);
	correlator->right = (void *)malloc(correlator->photon_size);
	correlator->scratch = (void *)malloc(correlator->photon_size);
			
	if ( correlator->photon_queue == NULL ||
			correlator->index_offsets == NULL ||
			correlator->photon_permutation == NULL ||
			correlator->current_correlation == NULL ||
			correlator->left == NULL ||
			correlator->right == NULL ||
			correlator->scratch == NULL ) {
		correlator_free(&correlator);
		return(correlator);
	}

	correlator->min_time_distance = options->min_time_distance;
	correlator->max_time_distance = options->max_time_distance;
	correlator->min_pulse_distance = options->min_pulse_distance;
	correlator->max_pulse_distance = options->max_pulse_distance;

	return(correlator);
}

int correlator_init(correlator_t *correlator, photon_stream_t *photon_stream,
		pc_options_t const *options) {
	if ( correlator->mode != photon_stream->mode ) {
		error("Photon stream mode does not match correlator (%d, %d)\n",
				photon_stream->mode, correlator->mode);
		return(PC_ERROR_MODE);
	}

	correlator->photon_stream = photon_stream;

	photon_queue_init(correlator->photon_queue);
	index_offsets_init(correlator->index_offsets, 0);
	permutation_init(correlator->photon_permutation);
	correlation_init(correlator->current_correlation);

	correlator->eof = 0;
	correlator->in_block = 0;
	correlator->in_permutations = 0;

	return(PC_SUCCESS);
}

int correlator_next(correlator_t *correlator) {
	int result;

	if ( correlator->order == 1 ) {
		result = photon_stream_next_photon(correlator->photon_stream);
		if ( result == PC_SUCCESS ) {
			correlation_set_index(correlator->current_correlation,
					correlator->photon_stream->current_photon,
					0);
			return(PC_SUCCESS);
		} else {
			return(result);
		}
	}

	while ( 1 ) {
		if ( correlator->in_block ) {
			debug("Yielding from the current block.\n");
			result = correlator_yield_from_block(correlator);

			if ( result == PC_SUCCESS ) {
				return(PC_SUCCESS);
			} else if ( result == PC_COMBINATION_OVERFLOW ) {
				// cycle around to the new block
				correlator->in_block = 0;
			} else {
				error("Unhandled error while yielding from the current "
						"correlation block: %d\n", result);
				return(result);
			}
		} else {
			debug("Populating a new block.\n");
			result = correlator_populate(correlator);

			if ( result == EOF || result == PC_WINDOW_NEXT ) {
				return(EOF);
			} else if ( result == PC_SUCCESS ) {
				correlator_block_init(correlator);
			} else if ( result == PC_ERROR_QUEUE_OVERFLOW ) {
				error("Queue overflow. Enlarge the queue to perform this "
						"calculation (current size: %zu)\n", 
						correlator->photon_queue->length);
				return(result);
			} else {
				error("Unhandled error while populating the correlator: %d\n",
						result);
				return(result);
			}
		}
	}
}

void correlator_free(correlator_t **correlator) {
	if ( *correlator != NULL ) {
		photon_queue_free(&((*correlator)->photon_queue));
		index_offsets_free(&((*correlator)->index_offsets));
		permutation_free(&((*correlator)->photon_permutation));
		correlation_free(&((*correlator)->current_correlation));
		free((*correlator)->left);
		free((*correlator)->right);
		free((*correlator)->scratch);
		free(*correlator);
	}
}

int correlator_populate(correlator_t *correlator) {
/* To populate the queue, first remove the front photon since it has been
 * used. Next, bring in new photons from the stream until we have surpassed the
 * maximal distance between two photons. If we have enough photons to perform
 * the correlation, yield PC_SUCCESS. If we have too few, pop the first photon
 * and keep going. If we reach the end of the stream, stop gathering photons
 * but pop the front sequentially until the queue is too small to correlate.
 */
	int result;

	photon_queue_pop(correlator->photon_queue,
			correlator->current_correlation->photons);

	while ( 1 ) {
		photon_queue_front(correlator->photon_queue, correlator->left);
		photon_queue_back(correlator->photon_queue, correlator->right);

		if ( photon_stream_eof(correlator->photon_stream) || 
				result == PC_WINDOW_NEXT ) {
			debug("EOF of photon stream\n");
			if ( photon_queue_size(correlator->photon_queue) >=
					correlator->order ) {
				debug("Enough to correlate.\n");
				return(PC_SUCCESS);
			} else {
				debug("Not enough to correlate.\n");
				return(EOF);
			}
		} else if ( photon_queue_size(correlator->photon_queue) >=
					correlator->order &&
				! correlator->under_max_distance(correlator) ) {
			debug("Over max distance, even after popping.\n");
			return(PC_SUCCESS);
		} else {
			debug("Getting a photon from the stream.\n");
			result = photon_stream_next_photon(correlator->photon_stream);

			if ( result == PC_SUCCESS ) {
				result = photon_queue_push(correlator->photon_queue,
						correlator->photon_stream->current_photon);
				if ( result != PC_SUCCESS ) {
					error("Error while adding photon to queue: %d\n", 
							result);
					return(result);
				} 
			} else if ( result == EOF || result == PC_WINDOW_NEXT ) {
				/* Loop around to deal with the existing photons. */
			} else {
				error("Unknown result of photon stream: %d\n", result);
				return(result);
			}
		}
	}
}

void correlator_block_init(correlator_t *correlator) {
	index_offsets_init(correlator->index_offsets, 
			photon_queue_size(correlator->photon_queue) - 1);
	permutation_init(correlator->photon_permutation);
	correlator->in_block = 1;
	correlator->in_permutations = 0;
}

int correlator_yield_from_block(correlator_t *correlator) {
	int result;

	while ( 1 ) {
		if ( correlator->in_permutations ) {
			/* Try to build the correlation from the current offsets and the
			 * next permutations.
			 */
			debug("Getting the next permutation.\n");
			result = permutation_next(correlator->photon_permutation);

			if ( result == PC_SUCCESS ) {
				/* valid permutation */
				return(correlator_build_correlation(correlator));
			} else if ( result == PC_COMBINATION_OVERFLOW ) {
				/* end of these offsets */
				debug("End of the current index offsets.\n");
				correlator->in_permutations = 0;
			} else {
				error("Unknown result of permutations: %d\n", result);
				return(result);
			}
		} else {
			debug("Getting the next index offsets.\n");
			result = index_offsets_next(correlator->index_offsets);

			if ( result == PC_SUCCESS ) {
				/* Possible valid offsets, check the distance. */
				if ( correlator_valid_distance(correlator) ) {
					debug("Valid offsets, moving to permutations.\n");
					permutation_init(correlator->photon_permutation);
					correlator->in_permutations = 1;
				} else {
					/* distance too far, disregard this one. */
				}
			} else if ( result == PC_COMBINATION_OVERFLOW ) {
				/* end of this block */
				debug("End of the current block.\n");
				return(PC_COMBINATION_OVERFLOW);
			} else {
				error("Unknown result of index offsets: %d\n", result);
				return(result);
			}
		}
	}
}

int correlator_valid_distance(correlator_t *correlator) {
	photon_queue_index(correlator->photon_queue,
			correlator->left,
			correlator->index_offsets->current_index_offsets->values[0]);
	photon_queue_index(correlator->photon_queue, 
			correlator->right,
			correlator->index_offsets->current_index_offsets->values[
				correlator->order-1]);
		
	return( correlator->over_min_distance(correlator) &&
			correlator->under_max_distance(correlator) );
}

int correlator_build_correlation(correlator_t *correlator) {
	int i;
	int result;
	correlation_t *cc = correlator->current_correlation;
	combination_t *io = correlator->index_offsets->current_index_offsets;
	permutation_t *p = correlator->photon_permutation;

	debug("Populating correlation.\n");
	for ( i = 0; i < correlator->order; i++ ) {
		result = photon_queue_index(correlator->photon_queue,
				&(cc->photons[i*correlator->photon_size]),
				io->values[p->values[i]]);

		if ( result != PC_SUCCESS ) {
			error("Bad index request during correlation: %d\n", 
					io->values[p->values[i]]);
			return(result);
		}
	}

	debug("Correlating photons.\n");
	correlator->correlate(correlator->current_correlation);

	return(PC_SUCCESS);
}

void correlator_flush(correlator_t *correlator) {
	photon_queue_flush(correlator->photon_queue);
}
