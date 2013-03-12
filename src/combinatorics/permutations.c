#include <stdlib.h>
#include <string.h>

#include "permutations.h"
#include "../error.h"

permutation_t *permutation_alloc(unsigned int const length, 
		int const positive_only) {
	unsigned int i;
	permutation_t *permutation = NULL;

	debug("Allocating permutation: %d length; positive only?: %d\n",
			length, positive_only);
	permutation = (permutation_t *)malloc(sizeof(permutation_t));

	if ( permutation == NULL ) {
		return(permutation);
	}

	permutation->positive_only = positive_only;
	permutation->length = length;
	permutation->populated = 0;

	permutation->n_permutations = n_permutations(length);
	debug("Allocating %d permutations.\n", permutation->n_permutations);

	permutation->permutations = (unsigned int **)malloc(
			permutation->n_permutations*sizeof(unsigned int *));
	permutation->scratch = combination_alloc(length, length);
	permutation->values = NULL;

	if ( permutation->permutations == NULL || permutation->scratch == NULL) {
		permutation_free(&permutation);
		return(permutation);
	}

	for ( i = 0; i < permutation->n_permutations; i++ ) {
		debug("Allocating combination %d\n", i);
		permutation->permutations[i] = (unsigned int *)malloc(
				sizeof(unsigned int)*length);
		if ( permutation->permutations[i] == NULL ) {
			permutation_free(&permutation);
			return(permutation);
		}
	}

	debug("Finished allocating permutation.\n");
	return(permutation);
}

void permutation_init(permutation_t *permutation) {
	unsigned int i;
	unsigned int j;
	int valid;

	if ( ! permutation->populated ) {
		debug("Initializing permuations.\n");

		for ( i = 0; i < permutation->n_permutations; i++ ) {
			memset(permutation->permutations[i], 
					0, 
					sizeof(unsigned int)*permutation->length);
		}
	
		debug("Initializing scratch combination.\n");
		combination_init(permutation->scratch);
	
		i = 0;
	
		while ( combination_next(permutation->scratch) == PC_SUCCESS ) {
			debug("Currently at combination %d\n", i);
			valid = is_permutation(permutation->scratch) &&
					! ( permutation->positive_only && 
						! is_positive_permutation(permutation->scratch));
	
			if ( valid ) {
				debug("Found a permutation.\n");
				for ( j = 0; j < permutation->length; j++ ) {
					permutation->permutations[i][j] =
							permutation->scratch->values[j];
				}
	
				i++;
			}
		}

		j = i;

		permutation->n_permutations = i;
		debug("Found %d permutation.\n", permutation->n_permutations);

		/* Free the excess permutations: we assume all permutations, but may
		 * only end up needing the positive ones.
		 */
		for ( i = j+1; i < j; i++ ) {
			free(permutation->permutations[i]);
		}

		permutation->populated = 1;
	}

	permutation->yielded = 0;
	permutation->values = permutation->permutations[0];
	permutation->current_index = 0;
}

int permutation_next(permutation_t *permutation) {
	if ( permutation->yielded ) {
		permutation->current_index++;
		if ( permutation->current_index >= permutation->n_permutations ) {
			return(PC_COMBINATION_OVERFLOW);
		} else {
			permutation->values = 
					permutation->permutations[permutation->current_index];
			permutation->yielded = 1;
			return(PC_SUCCESS);
		}
	} else {
		permutation->yielded = 1;
		permutation->values = 
				permutation->permutations[permutation->current_index];
		return(PC_SUCCESS);
	}
}

void permutation_free(permutation_t **permutation) {
	int i;
	if ( *permutation != NULL ) {
		for ( i = 0; (*permutation)->permutations != NULL && 
				i < (*permutation)->n_permutations; i++ ) {
			free((*permutation)->permutations[i]);
		}

		free((*permutation)->permutations);
		combination_free(&((*permutation)->scratch));
		free(*permutation);
	}
}

int permutation_fprintf(FILE *stream_out, permutation_t const *permutation) {
	int i;

	for ( i = 0; i < permutation->length; i++ ) {
		fprintf(stream_out, "%u", permutation->values[i]);
		if ( i+1 == permutation->length ) {
			fprintf(stream_out, "\n");
		} else {
			fprintf(stream_out, ",");
		}
	}

	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
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
