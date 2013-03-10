#include <stdlib.h>
#include <string.h>

#include "vector.h"
#include "../error.h"

vector_t *vector_alloc (size_t const elem_size, size_t const length) {
	vector_t *vector = (vector_t *)malloc(sizeof(vector_t));

	if ( vector == NULL ) {
		return(vector);
	}

	vector->length = length;
	vector->empty = 1;
	vector->left_index = 0;
	vector->right_index = 0;
	
	vector->elem_size = elem_size;
	vector->compare = NULL;

	vector->values = malloc(vector->elem_size*length);

	if ( vector->values == NULL ) {
		vector_free(&vector);
		return(vector);
	}

	return(vector);
}

void vector_init(vector_t *vector) {
	vector->empty = 1;
	vector->left_index = 0;
	vector->right_index = 0;
}

void vector_free(vector_t **vector) {
	if ( *vector != NULL ) {
		free((*vector)->values);
		free(*vector);
		*vector = NULL;
	}
}

int vector_full(vector_t const *vector) {
	return(vector_size(vector) >=  vector->length);
}

int vector_empty(vector_t const *vector) {
	return(vector_size(vector) == 0);
}

size_t vector_size(vector_t const *vector) {
	if ( vector->empty ) {
		return(0);
	} else {
		return(vector->right_index - vector->left_index + 1);
	}
}

size_t vector_capacity(vector_t const *vector) {
	return(vector->length);
}

int vector_resize(vector_t *vector, size_t const length) {
	return(PC_ERROR_MEM);
}

void vector_set_comparator(vector_t *vector, compare_t compare) {
	vector->compare = compare;
}

int vector_sort(vector_t *vector) {
	/* First, check if the vector wraps around. If it does, we need to move 
	 * everything into one continous block. If it does not, it is already in 
	 * a continuous block and is ready for sorting.
	 */
	size_t right = vector->right_index % vector->length;
	size_t left = vector->left_index % vector->length;

	if ( vector->compare == NULL ) {
		error("No comparator installed for this vector.\n");
		return(PC_ERROR_OPTIONS);
	}

	if ( vector_full(vector) ) {
		debug("Vector is full, no action is needed to make it contiguous for "
				"sorting.\n");
	} else {
		if ( right < left ) {
			/* The vector wraps around, so join the two together by moving the 
			 * right-hand bit over. 
			 * xxxxx---yyyy -> xxxxxyyyy---
			 */
			debug("Vector loops around, moving records to "
					"make them contiguous.\n");
			memmove(&(vector->values[(right+1)*vector->elem_size]), 
					&(vector->values[left*vector->elem_size]),
					vector->elem_size*(vector->length - left));
		} else if ( left != 0 && ! vector_full(vector) ) {
			/* There is one continuous block, so move it to the front. 
			 * ---xxxx -> xxxx---
			 */
			debug("Vector is offset from beginning, moving it forward.\n");
			memmove(vector->values,
					&(vector->values[left*vector->elem_size]),
					vector->elem_size*vector_size(vector));
		} else {
			debug("Vector starts at the beginning of the array, "
					"no action needed to make it contiguous.\n");
		}
	}

	vector->right_index = vector_size(vector) - 1;
	vector->left_index = 0;

	qsort(vector->values, 
			vector_size(vector), 
			vector->elem_size, 
			vector->compare); 

	return(PC_SUCCESS);
}

int vector_pop(vector_t *vector, void *elem) {
	int result = vector_front(vector, elem);

	if ( result == PC_SUCCESS ) {
		vector->left_index++;
		if ( vector->left_index > vector->right_index ) {
			vector_init(vector);
		}
	}

	return(result);
}

int vector_push(vector_t *vector, void const *elem) {
	int result;
	size_t next_index;

	if ( vector_full(vector) ) {
		debug("Vector overflowed its bounds (current capacity %zu).\n",
				vector_capacity(vector));

		/* Check for integer overflow */
		if ( vector->length * 2 > vector->length ) {
			result = vector_resize(vector, vector->length * 2);

			if ( result != PC_SUCCESS ) {
				error("Could not resize the vector after an overflow.\n");
				return(result);
			}
		} else {
			return(PC_ERROR_MEM);
		}
	}

	if ( vector->empty ) {
		vector->right_index = 0;
		vector->left_index = 0;
		next_index = 0;
	} else {
		next_index = (vector->right_index + 1) % vector->length;
	}
	
	memcpy(&(vector->values[next_index*vector->elem_size]), 
			elem, 
			vector->elem_size);

	if ( vector->empty ) {
/* 
 * For the empty vector, there is no need to expand the bounds. 
 * Just mark it not empty. 
 */
		vector->empty = 0;
	} else {
		vector->right_index++;
	}

	return(PC_SUCCESS);
}

int vector_index(vector_t const *vector, void *elem, size_t const index) {
	size_t true_index = (vector->left_index + index) % vector->length;

	if ( index > vector_size(vector) ) {
		return(PC_ERROR_INDEX);
	} else { 
		memcpy(elem, 
				&(vector->values[true_index*vector->elem_size]), 
				vector->elem_size);

		return(PC_SUCCESS);
	}
}

int vector_front(vector_t const *vector, void *elem) {
	return(vector_index(vector, elem, 0));
}

int vector_back(vector_t const *vector, void *elem) {
	return(vector_index(vector, elem, vector_size(vector)-1));
}
