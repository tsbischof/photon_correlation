#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "error.h"

queue_t *queue_alloc (size_t const elem_size, size_t const length) {
	queue_t *queue = (queue_t *)malloc(sizeof(queue_t));

	if ( queue == NULL ) {
		return(queue);
	}

	queue->length = length;
	queue->empty = 1;
	queue->left_index = 0;
	queue->right_index = 0;
	
	queue->elem_size = elem_size;
	queue->compare = NULL;

	queue->values = malloc(queue->elem_size*length);

	if ( queue->values == NULL ) {
		queue_free(&queue);
		return(queue);
	}

	return(queue);
}

void queue_init(queue_t *queue) {
	queue->empty = 1;
	queue->left_index = 0;
	queue->right_index = 0;
}

void queue_free(queue_t **queue) {
	if ( *queue != NULL ) {
		free((*queue)->values);
		free(*queue);
		*queue = NULL;
	}
}

int queue_full(queue_t const *queue) {
	return(queue_size(queue) >=  queue->length);
}

int queue_empty(queue_t const *queue) {
	return(queue_size(queue) == 0);
}

size_t queue_size(queue_t const *queue) {
	if ( queue->empty ) {
		return(0);
	} else {
		return(queue->right_index - queue->left_index + 1);
	}
}

size_t queue_capacity(queue_t const *queue) {
	return(queue->length);
}

int queue_resize(queue_t *queue, size_t const length) {
	size_t true_size = length * queue->elem_size;
	void *new;

	if ( length <= queue_capacity(queue) ) {
		error("Resize would shrink queue, skipping.\n");
		return(PC_ERROR_MEM);
	} else if ( true_size < length ) {
		error("Integer overflow when calculating new queue size.\n");
		return(PC_ERROR_MEM);
	} else {
		new = realloc(queue->values, true_size);

		if ( new == NULL ) {
			error("Could not realloc space to length %zu\n", length);
			return(PC_ERROR_MEM);
		} else {
			queue->values = new;
			queue->length = length;
			return(PC_SUCCESS);
		}
	}
}

void queue_set_comparator(queue_t *queue, compare_t compare) {
	queue->compare = compare;
}

int queue_sort(queue_t *queue) {
	/* First, check if the queue wraps around. If it does, we need to move 
	 * everything into one continous block. If it does not, it is already in 
	 * a continuous block and is ready for sorting.
	 */
	size_t right = queue->right_index % queue->length;
	size_t left = queue->left_index % queue->length;

	if ( queue->compare == NULL ) {
		error("No comparator installed for this queue.\n");
		return(PC_ERROR_OPTIONS);
	}

	if ( queue_full(queue) ) {
		debug("Vector is full, no action is needed to make it contiguous for "
				"sorting.\n");
	} else {
		if ( right < left ) {
			/* The queue wraps around, so join the two together by moving the 
			 * right-hand bit over. 
			 * xxxxx---yyyy -> xxxxxyyyy---
			 */
			debug("Vector loops around, moving records to "
					"make them contiguous.\n");
			memmove(&(((char *)queue->values)[(right+1)*queue->elem_size]), 
					&(((char *)queue->values)[left*queue->elem_size]),
					queue->elem_size*(queue->length - left));
		} else if ( left != 0 && ! queue_full(queue) ) {
			/* There is one continuous block, so move it to the front. 
			 * ---xxxx -> xxxx---
			 */
			debug("Vector is offset from beginning, moving it forward.\n");
			memmove(queue->values,
					&(((char *)queue->values)[left*queue->elem_size]),
					queue->elem_size*queue_size(queue));
		} else {
			debug("Vector starts at the beginning of the array, "
					"no action needed to make it contiguous.\n");
		}
	}

	queue->right_index = queue_size(queue) - 1;
	queue->left_index = 0;

	qsort(queue->values, 
			queue_size(queue), 
			queue->elem_size, 
			queue->compare); 

	return(PC_SUCCESS);
}

int queue_pop(queue_t *queue, void *elem) {
	int result = queue_front(queue, elem);

	if ( result == PC_SUCCESS ) {
		queue->left_index++;
		if ( queue->left_index > queue->right_index ) {
			queue_init(queue);
		}
	}

	return(result);
}

int queue_push(queue_t *queue, void const *elem) {
	int result;
	size_t next_index;

	if ( queue_full(queue) ) {
		warn("Queue needs to be expanded. It may be worthwhile to "
				"perform this at the start of the calculation instead by "
				"passing --queue-size.\n");
		debug("Vector overflowed its bounds (current capacity %zu).\n",
				queue_capacity(queue));

		/* Check for integer overflow */
		if ( queue->length * 2 > queue->length ) {
			result = queue_resize(queue, queue->length * 2);

			if ( result != PC_SUCCESS ) {
				error("Could not resize the queue after an overflow.\n");
				return(result);
			}
		} else {
			error("Queue resize would cause integer overflow: %zu -> %zu\n",
					queue->length, queue->length * 2);
			return(PC_ERROR_MEM);
		}
	}

	if ( queue->empty ) {
		queue->right_index = 0;
		queue->left_index = 0;
		next_index = 0;
	} else {
		next_index = (queue->right_index + 1) % queue->length;
	}
	
	memcpy(&(((char *)queue->values)[next_index*queue->elem_size]), 
			elem, 
			queue->elem_size);

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

int queue_index(queue_t const *queue, void *elem, size_t const index) {
	size_t true_index = (queue->left_index + index) % queue->length;

	if ( index >= queue_size(queue) ) {
		return(PC_ERROR_INDEX);
	} else { 
		memcpy(elem, 
				&(((char *)queue->values)[true_index*queue->elem_size]), 
				queue->elem_size);

		return(PC_SUCCESS);
	}
}

int queue_front(queue_t const *queue, void *elem) {
	return(queue_index(queue, elem, 0));
}

int queue_back(queue_t const *queue, void *elem) {
	return(queue_index(queue, elem, queue_size(queue)-1));
}
