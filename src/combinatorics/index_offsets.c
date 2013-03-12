#include <stdio.h>
#include <stdlib.h>

#include "index_offsets.h"
#include "../error.h"

index_offsets_t *index_offsets_alloc(unsigned int const length) {
	index_offsets_t *index_offsets = NULL;

	index_offsets = (index_offsets_t *)malloc(sizeof(index_offsets_t));

	if ( index_offsets == NULL ) {
		return(index_offsets);
	} 

	index_offsets->length = length;
	index_offsets->current_index_offsets = combination_alloc(length, length);

	if ( index_offsets->current_index_offsets == NULL ) {
		index_offsets_free(&index_offsets);
		return(index_offsets);
	} 

	return(index_offsets);
}

void index_offsets_init(index_offsets_t *index_offsets, 
		unsigned int const limit) {
	int i;
	index_offsets->limit = limit;

	combination_init(index_offsets->current_index_offsets);
	index_offsets->yielded = 0;

	for ( i = 0; i < index_offsets->length; i++ ) {
		index_offsets->current_index_offsets->values[i] = i;
	}
}

int index_offsets_next(index_offsets_t *index_offsets) {
	int i;
	unsigned int leading_digit;
	combination_t *current = index_offsets->current_index_offsets;

	if ( index_offsets->yielded ) {
		leading_digit = current->values[1];

		for ( i = current->length - 1; i > 0; i-- ) { 
			current->values[i]++;
			if ( current->values[i] <= index_offsets->limit ) {
				/* No overflow */
				i = 0;
			} else {
				current->values[i] = 0;
			}
		}
	
		if ( current->values[1] == 0 ) {
			current->values[1] = leading_digit+1;
		}
	
		for ( i = 1; i < current->length; i++ ) {
			if ( current->values[i] == 0 ) {
				current->values[i] = current->values[i-1] + 1;
			} else {
				/* nothing, because we already incremented. */
				;
			}

			if ( current->values[i] > index_offsets->limit ) {
				return(PC_COMBINATION_OVERFLOW);
			}
		}

		return(PC_SUCCESS);
	} else {
		index_offsets->yielded = 1;
		return(PC_SUCCESS);
	}
}

void index_offsets_free(index_offsets_t **index_offsets) {
	if ( *index_offsets != NULL ) {
		combination_free(&((*index_offsets)->current_index_offsets));
		free(*index_offsets);
	}
}
