#include <stdlib.h>

#include "permutations.h"
#include "error.h"

permutations_t *permutations_alloc(unsigned int const length, 
		int const positive_only) {
	unsigned int i;

	permutations_t *permutations = NULL;

	permutations = (permutations_t *)malloc(sizeof(permutations_t));

	if ( permutations == NULL ) {
		return(permutations);
	}

	permutations->positive_only = positive_only;
	permutations->length = length;

	permutations->n_permutations = n_permutations(length);

	permutations->permutations = (combination_t **)malloc(
			sizeof(combination_t *)*permutations->n_permutations);
	permutations->scratch = combinations_alloc(length, length);

	if ( permutations->permutations == NULL || permutations->scratch == NULL) {
		permutations_free(&permutations);
		return(permutations);
	}

	for ( i = 0; i < length; i++ ) {
		permutations->permutations[i] = combination_alloc(length);
		if ( permutations->permutations[i] == NULL ) {
			permutations_free(&permutations);
			return(permutations);
		}
	}

	return(permutations);
}

void permutations_init(permutations_t *permutations) {
	unsigned int i;
	unsigned int j;
	int valid;

	combination_t *current;

	permutations->yielded = 0;
	permutations->current_index = 0;

	for ( i = 0; i < permutations->n_permutations; i++ ) {
		combination_init(permutations->permutations[i]);
	}

	combinations_init(permutations->scratch);

	i = 0;

	while ( combinations_next(permutations->scratch) ) {
		current = permutations->scratch->current_combination;
		valid = is_permutation(current) &&
				! ( permutations->positive_only && 
					! is_positive_permutation(current));

		if ( valid ) {
			for ( j = 0; j < current->length; j++ ) {
				permutations->permutations[i]->values[j] =
						current->values[j];
			}
		}
	}

	permutations->n_permutations = i+1;
}

int permutations_next(permutations_t *permutations) {
	if ( permutations->yielded ) {
		permutations->current_index++;
		if ( permutations->current_index >= permutations->n_permutations ) {
			return(PC_COMBINATION_OVERFLOW);
		} else {
			permutations->current_permutation = 
					permutations->permutations[permutations->current_index];
			permutations->yielded = 1;
			return(PC_SUCCESS);
		}
	} else {
		permutations->yielded = 1;
		permutations->current_permutation = 
				permutations->permutations[permutations->current_index];
		return(PC_SUCCESS);
	}
}

void permutations_free(permutations_t **permutations) {
	int i;
	if ( *permutations != NULL ) {
		for ( i = 0; (*permutations)->permutations != NULL && 
				i < (*permutations)->n_permutations; i++ ) {
			combination_free(&((*permutations)->permutations[i]));
		}

		free((*permutations)->permutations);
		combinations_free(&((*permutations)->scratch));
		free(*permutations);
	}
}

int is_permutation(combination_t const *combination) {
	unsigned int i;
	unsigned int j;

	for ( i = 0; i < combination->length; i++ ) {
		for ( j = i+1; j < combination->length; j++ ) {
			if ( combination->values[i] == combination->values[j] ) {
				return(0);
			}
		}
	}

	return(1);
}

int is_positive_permutation(combination_t const *combination) {
	unsigned int i;

	if ( ! is_permutation(combination) ) {
		return(0);
	}

	for ( i = 0; i < combination->length-1; i++ ) {
		if ( combination->values[i] >= combination->values[i+1] ) {
			return(0);
		}
	}

	return(1);
}
