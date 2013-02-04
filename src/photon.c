#include <stdlib.h>
#include <string.h>

#include "photon.h"
#include "error.h"
#include "t2.h"
#include "t2_void.h"
#include "t3.h"
#include "t3_void.h"
#include "modes.h"
/* 
 * There are three main use cases for a photon stream:
 * (1) Bulk transfer. In channels, this is used to read and sort large numbers
 *     of photons rapidly.
 * (2) Photon-by-photon processing. In correlate, this is used to ensure that
 *     no more photons are pulled into memory than necessary.
 * (3) Photon-window processing. In intensity, this is used to provide a simple
 *     way of segregating the photons for counting. 
 * 
 * Given these constraints, (1) can be taken care of with the elementary photon
 * read/write routines, or with bulk equivalents if desired. For (3), a 
 * windowed stream, where the current time limits are tracked and iterated over
 * as needed does a good job of providing an iterable interface for users. 
 * Less obviously, (2) can use the same interface, since the window is now
 * infinite. To avoid unnecessary checks of the window bounds, a separate
 * iteration routine can be used which does not include that logic. 
 * 
 * To implement the stream, we need a few things:
 * (1) An incoming, unformatted stream (FILE *)
 * (2) A window (lower, upper)
 * (3) The last photon seen and a flag indicating whether it has been yielded.
 *
 * (3) is only necessary in the windowed case, since it is possible to reach
 * the end of a time bin before the photon can be emitted (and in fact, this
 * is the whole point of the operation). This is the logic we will be avoiding
 * by using a separate iteration routine in use case (2).
 *
 * The basic algorithm for the photon stream goes as follows:
 * window = next(windows)
 * current = next(photons)
 * yielded = False
 * while (photons is not empty) or (not yielded):
 *     if not yielded:
 *         if current in window:
 *             yield(current)
 *             yielded = True
 *         else:
 *             window = next(windows)
 *     else:
 *         current = next(photons)
 *         yielded = False
 * 
 * The non-windowed version is much simpler:
 * while photons is not empty:
 *     yield(next(photons))
 */
photon_queue_t *photon_queue_alloc (size_t const length, int const mode) {
	photon_queue_t *queue = NULL;

	debug("Allocating photon queue, length %zu, mode %d\n", length, mode);
	queue = (photon_queue_t *)malloc(sizeof(photon_queue_t));

	if ( queue == NULL ) {
		return(queue);
	}

	queue->length = length;
	queue->left_index = 0;
	queue->right_index = 0;
	queue->empty = 1;

	queue->mode = mode;

	if ( queue->mode == MODE_T2 ) {
		queue->photon_size = sizeof(t2_t);
		queue->compare = t2_compare;
	} else if ( queue->mode == MODE_T3 ) {
		queue->photon_size = sizeof(t3_t);
		queue->compare = t3_compare;
	} else {
		photon_queue_free(&queue);
		return(queue);
	}

	debug("Queue photon size: %zu\n", queue->photon_size);
	
	queue->queue = malloc(queue->photon_size*length);

	if ( queue->queue == NULL ) {
		photon_queue_free(&queue);
		return(queue);
	}

	debug("Done allocating photon queue.\n");

	return(queue);
}

void photon_queue_init(photon_queue_t *queue) {
	if ( queue->mode == MODE_T2 ) {
		queue->compare = t2_compare;
	} else if ( queue->mode == MODE_T3 ) {
		queue->compare = t3_compare;
	}
}

void photon_queue_free(photon_queue_t **queue) {
	if ( *queue != NULL ) {
		free((*queue)->queue);
		free(*queue);
		*queue = NULL;
	}
}

int photon_queue_full(photon_queue_t const *queue) {
	return( photon_queue_size(queue) >=  queue->length );
}

int photon_queue_empty(photon_queue_t const *queue) {
	return( photon_queue_size(queue) == 0 );
}

int photon_queue_pop(photon_queue_t *queue, void *photon) {
	int result = photon_queue_front(queue, photon);

	if ( result == PC_SUCCESS ) {
		queue->left_index++;
		if ( queue->left_index > queue->right_index ) {
			queue->empty = 1;
		}
	}

	return(result);
}

int photon_queue_push(photon_queue_t *queue, void *photon) {
	size_t next_index;

	if ( photon_queue_full(queue) ) {
		return(PC_ERROR_QUEUE_OVERFLOW);
	}

	if ( queue->empty ) {
		queue->right_index = 0;
		queue->left_index = 0;
		next_index = 0;
	} else {
		next_index = (queue->right_index + 1) % queue->length;
	}
	
	memcpy(&(queue->queue[next_index*queue->photon_size]), 
			photon, 
			queue->photon_size);

	if ( queue->empty ) {
/* 
 * For the empty queue, there is no need to expand the bounds. 
 * Just mark it not empty. 
 */
		queue->empty = 0;
	} else {
		queue->right_index++;
	}

	return(PC_SUCCESS);
}

int photon_queue_front(photon_queue_t const *queue, void *photon) {
	return(photon_queue_index(queue, photon, 0));
}

int photon_queue_back(photon_queue_t const *queue, void *photon) {
	return(photon_queue_index(queue, photon, photon_queue_size(queue)-1));
}

int photon_queue_index(photon_queue_t const *queue, void *photon, 
		size_t const index) {
	size_t true_index = (queue->left_index + index) % queue->length;

	if ( index > photon_queue_size(queue) ) {
		return(PC_ERROR_QUEUE_INDEX);
	} else { 
		memcpy(photon, 
				&(queue->queue[true_index*queue->photon_size]), 
				queue->photon_size);

		return(PC_SUCCESS);
	}
}

size_t photon_queue_size(photon_queue_t const *queue) {
	if ( queue->empty ) {
		return(0);
	} else {
		return(queue->right_index - queue->left_index + 1);
	}
}

void photon_queue_sort(photon_queue_t *queue) {
	/* First, check if the queue wraps around. If it does, we need to move 
	 * everything into one continous block. If it does not, it is already in 
	 * a continuous block and is ready for sorting.
	 */
	size_t right = queue->right_index % queue->length;
	size_t left = queue->left_index % queue->length;
	if ( photon_queue_full(queue) ) {
		debug("Queue is full, no action is needed to make it contiguous for "
				"sorting.\n");
	} else {
/*		warn("Sorting with a non-full queue is not thoroughly tested. "
				"Use these results at your own risk.\n"); */
		if ( right < left ) {
			/* The queue wraps around, so join the two together by moving the 
			 * right-hand bit over. 
			 * xxxxx---yyyy -> xxxxxyyyy---
			 */
			debug("Queue loops around, moving records to "
					"make them contiguous.\n");
			memmove(&(queue->queue[(right+1)*queue->photon_size]), 
					&(queue->queue[left*queue->photon_size]),
					queue->photon_size*(queue->length - left));
		} else if ( left != 0 && ! photon_queue_full(queue) ) {
			/* There is one continuous block, so move it to the front. 
			 * ---xxxx -> xxxx---
			 */
			debug("Queue is offset from beginning, moving it forward.\n");
			memmove(queue->queue,
					&(queue->queue[left*queue->photon_size]),
					queue->photon_size*photon_queue_size(queue));
		} else {
			debug("Queue starts at the beginning of the array, "
					"no action needed to make it contiguous.\n");
		}
	}

	queue->right_index = photon_queue_size(queue) - 1;
	queue->left_index = 0;

	qsort(queue->queue, 
			photon_queue_size(queue), 
			queue->photon_size, 
			queue->compare); 
}

/* 
 * Implementation of the photon stream (cases 2 and 3)
 */
void photon_window_init(photon_window_t *window, options_t const *options) {
	window->limits.lower = options->start_time;
	window->limits.upper = window->limits.lower + options->bin_width;
	window->width = options->bin_width;
	window->set_lower_bound = options->set_start_time;
	window->set_upper_bound = options->set_stop_time;
	window->upper_bound = options->stop_time;

	if ( options->set_stop_time && 
		window->limits.upper > options->stop_time ) {
		window->limits.upper = options->stop_time;
	}
}

int photon_window_next(photon_window_t *window) {
	window->limits.lower += window->width;
	window->limits.upper += window->width;

	if ( window->set_upper_bound && 
			window->limits.upper > window->upper_bound ) {
		if ( window->limits.lower < window->upper_bound ) {
			window->limits.upper = window->upper_bound;
			/* Still within the bound, just trim it down a bit. */
			return(PC_SUCCESS);
		} else {
			/* Outside the upper bound. */
			return(PC_WINDOW_EXCEEDED);
		}
	} else {
		return(PC_SUCCESS);
	}
}

int photon_window_contains(photon_window_t const *window, int64_t const value) {
	return( window->limits.lower <= value &&
			window->limits.upper > value );
}

photon_stream_t *photon_stream_alloc(options_t const *options) {
	photon_stream_t *photons = NULL;

	photons = (photon_stream_t *)malloc(sizeof(photon_stream_t));

	if ( photons == NULL ) {
		return(photons);
	}

	/* Leave space here to allocate a photon queue. */
	return(photons);	
}

int photon_stream_init(photon_stream_t *photons, FILE *stream_in, 
		options_t const *options) {
	photons->mode = options->mode;

	if ( options->mode == MODE_T2 ) {
		photons->photon_size = sizeof(t2_t);
		photons->photon_next = T2V_NEXT(options->binary_in);
		photons->photon_print = T2V_PRINT(options->binary_out);
		photons->window_dim = t2v_window_dimension;
		photons->channel_dim = t2v_channel_dimension;
	} else if ( options->mode == MODE_T3 ) {
		photons->photon_size = sizeof(t3_t);
		photons->photon_next = T3V_NEXT(options->binary_in);
		photons->photon_print = T3V_PRINT(options->binary_out);
		photons->window_dim = t3v_window_dimension;
		photons->channel_dim = t3v_channel_dimension;
	} else {
		return(PC_ERROR_MODE);
	}

	photons->current_photon = malloc(photons->photon_size);

	if ( photons->current_photon == NULL ) {
		return(PC_ERROR_MEM);
	}

	photon_window_init(&(photons->window), options);

	if ( options->set_start_time || options->set_stop_time ||
			options->bin_width != 0 ) {
		photons->photon_stream_next = photon_stream_next_windowed;
	} else {
		photons->photon_stream_next = photon_stream_next_unbounded;
	}

	photons->yielded_photon = 1; 
	/* If the photon has been yielded, the stream is reset and ready to 
	 * acquire a new one. Start in this state to pick up a new photon.
	 */
	photons->stream_in = stream_in;

	return(PC_SUCCESS);
}

void photon_stream_free(photon_stream_t **photons) {
	if ( *photons != NULL ) {
		free((*photons)->current_photon);
		free(*photons);
	}
}

int photon_stream_next_windowed(void *photon_stream) {
	photon_stream_t *photons = (photon_stream_t *)photon_stream;
	int64_t dim;
	int result;

	while ( 1 ) {
		if ( ! photons->yielded_photon ) {
			/* Photon available, check if it is in the window. */
			dim =  photons->window_dim(photons->current_photon);
			if ( photon_window_contains(&(photons->window), dim) ) {
				/* Found a photon, and in the window. */
				photons->yielded_photon = 1;
				return(PC_SUCCESS);
			} else if ( photons->window.limits.lower > dim ) {	
				/* Have photon, but window is ahead of it. Ignore this photon
				 * and loop until we find another.
				 * Status would be PC_WINDOW_AHEAD
				 */
				debug("Found a photon, but it was before the window.\n");
				photons->yielded_photon = 1;
			} else {
				/* Past the upper bound. */
				if ( photons->window.set_upper_bound && 
						dim >= photons->window.upper_bound ) {
					return(EOF);
				} else {
					/* Tell the caller to advance the window. */
					return(PC_WINDOW_NEXT);
				}
			}
		} else {
			result = photons->photon_next(photons->stream_in, 
					photons->current_photon);

			if ( result == PC_SUCCESS ) {
				/* Found one, loop back to see where it falls. */
				photons->yielded_photon = 0;
			} else if ( result == EOF ) {
				return(EOF);
			} else {
				/* An error of some nature. */
				error("Unhandled error while processing photon stream: %d.\n",
						result);
				return(result);
			}
		}
	}
}

int photon_stream_next_unbounded(void *photon_stream) {
	photon_stream_t *photons = (photon_stream_t *)photon_stream;
	return(photons->photon_next(photons->stream_in, photons->current_photon));
}

int photon_stream_next_window(photon_stream_t *photons) {
	return(photon_window_next(&(photons->window)));
}

int photon_stream_next_photon(photon_stream_t *photon_stream) {
	return(photon_stream->photon_stream_next(photon_stream));
}

int photon_stream_eof(photon_stream_t *photons) {
	return(feof(photons->stream_in));
}

int photon_echo(FILE *stream_in, FILE *stream_out, options_t const *options) {
	int result;
	photon_stream_t *photon_stream = photon_stream_alloc(options);

	if ( photon_stream == NULL ) {
		return(PC_ERROR_MEM);
	}

	result = photon_stream_init(photon_stream, stream_in, options);

	if ( result != PC_SUCCESS ) {
		return(result);
	}

	while ( photon_stream_next_photon(photon_stream) == PC_SUCCESS) {
		photon_stream->photon_print(stream_out, photon_stream->current_photon);
	}

	photon_stream_free(&photon_stream);
	return(PC_SUCCESS);
}
