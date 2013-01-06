#include <stdlib.h>
#include <string.h>

#include "photon.h"
#include "error.h"

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
photon_queue_t *photon_queue_alloc (size_t length, size_t photon_size) {
	photon_queue_t *queue;

	queue = (photon_queue_t *)malloc(sizeof(photon_queue_t));
	if ( queue != NULL ) {
		queue->length = length;
		queue->left_index = 0;
		queue->right_index = 0;
		queue->empty = 1;
		queue->photon_size = photon_size;
	
		queue->queue = (void *)malloc(photon_size*length);
		if ( queue->queue == NULL ) {
			photon_queue_free(&queue);
		}
	}

	return(queue);
}

void photon_queue_free (photon_queue_t **queue) {
	if ( *queue != NULL ) {
		free((*queue)->queue);
		free(*queue);
		*queue = NULL;
	}
}

int photon_queue_full(photon_queue_t *queue) {
	return( photon_queue_size(queue) >=  queue->length );
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

int photon_queue_front(photon_queue_t *queue, void *photon) {
	size_t index;

	if ( queue->empty ) {
		return(PC_ERROR_QUEUE_EMPTY);
	} else {
		index = queue->left_index % queue->length;
		memcpy(photon, 
				&(queue->queue[index*queue->photon_size]),
				queue->photon_size);
		return(PC_SUCCESS);
	}
}

int photon_queue_back(photon_queue_t *queue, void *photon) {
	size_t index;

	if ( queue->empty ) {
		return(PC_ERROR_QUEUE_EMPTY);
	} else {
		index = queue->right_index % queue->length;
		memcpy(photon,
				&(queue->queue[index]),
				queue->photon_size);
		return(PC_SUCCESS);
	}
}

size_t photon_queue_index(photon_queue_t *queue, void *photon, int index) {
	size_t true_index = (queue->left_index + index) % queue->length;

	if ( index > photon_queue_size(queue) ) {
		return(PC_ERROR_QUEUE_INDEX);
	} else { 
		memcpy(photon, 
				&(queue->queue[true_index]), 
				queue->photon_size);
		return(PC_SUCCESS);
	}
}

size_t photon_queue_size(photon_queue_t *queue) {
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
		warn("Sorting with a non-full queue is not thoroughly tested. "
				"Use these results at your own risk.\n");
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
void photon_window_init(photon_window_t *window, 
		int set_lower_bound, int64_t lower_bound, 
		uint64_t width,
		int set_upper_bound, int64_t upper_bound) {
	window->limits.lower = lower_bound;
	window->limits.upper = window->limits.lower + width;
	window->width = width;
	window->set_lower_bound = set_lower_bound;
	window->set_upper_bound = set_upper_bound;
	window->upper_bound = upper_bound;
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
			printf("here\n");
			return(PC_WINDOW_EXCEEDED);
		}
	} else {
		return(PC_SUCCESS);
	}
}

int photon_stream_init(photon_stream_t *photons, 
		photon_stream_next_t *stream_next,
		photon_window_dimension_t dim,
		photon_next_t photon_next,
		size_t photon_size, FILE *stream_in,
		int set_lower_bound, int64_t lower_bound, 
		uint64_t width,
		int set_upper_bound, int64_t upper_bound) {	
	photons->dim = dim;
	photons->photon_size = photon_size;
	photons->current_photon = malloc(photon_size);

	if ( photons->current_photon == NULL ) {
		return(PC_ERROR_MEM);
	}

	photon_window_init(&(photons->window),
			set_lower_bound, lower_bound, 
			width, 
			set_upper_bound, upper_bound);

	if ( set_upper_bound || width != 0 ) {
		*stream_next = photon_stream_next_windowed;
	} else {
		*stream_next = photon_stream_next_unbounded;
	}

	photons->photon_next = photon_next;
	photons->yielded_photon = 1; 
/* If the photon has been yielded, the stream is reset and ready to 
 * acquire a new one.
 */
	photons->stream_in = stream_in;

	return(PC_SUCCESS);
}

void photon_stream_free(photon_stream_t *photons) {
	free(photons->current_photon);
}

int photon_stream_next_windowed(photon_stream_t *photons, void *photon) {
	int64_t dim;
	int result;

	while ( 1 ) {
		if ( ! photons->yielded_photon ) {
			/* Photon available, check if it is in the window. */
			dim =  photons->dim(photons->current_photon);
			if ( photons->window.limits.lower <= dim 
					&& dim < photons->window.limits.upper ) {
				/* Found a photon, and in the window. */
				memcpy(photon,
						photons->current_photon,
						photons->photon_size);
				photons->yielded_photon = 1;
				return(PC_SUCCESS);
			} else if ( photons->window.limits.lower > dim ) {	
				/* Have photon, but window is ahead of it. Catch this to avoid
				 * an infinite loop.
				 */
				return(PC_WINDOW_AHEAD);
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

int photon_stream_next_unbounded(photon_stream_t *photons, void *photon) {
	int result;

	result = photons->photon_next(photons->stream_in, photon);

	return(result);
}

int photon_stream_next_window(photon_stream_t *photons) {
	return(photon_window_next(&(photons->window)));
}
