#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "combinations.h"

/* Set this to 1 to show all of the tuples of (channel, index) and whatnot. 
 * This is only really necessary for a full-scale debug, and should not be 
 * shown for a simple --verbose run.
 */
#define PRINT_TUPLES 0


/*
 *
 * Helper functions.
 *
 */
int pow_int(int base, int power) {
	int result = 1;
	int previous = result;
	int i;

	for ( i = 0; i < power; i++ ) {
		result *= base;
		if ( previous > result ) {
			/* Integer overflow. */
			error("Integer overflow when calculating %d^%d.\n", base, power);
			return(0);
		}
	}

	return(result);
}

int n_combinations(int channels, int order) {
	/* The number of combinations will be channels^order, since we have
 	 * order many digits to populate with a channel (up to channels-1)
 	 */
	return(pow_int(channels, order));
}

int n_permutations(int order) { 
	return(factorial(order));
}

int factorial(int n) {
	int i;
	int result = 1;

	for ( i = 1; i <= n; i++ ) {
		result *= i;
	}

	return(result);
}

/*
 *
 * Functions to deal with individual combinations.
 *
 */
combination_t *allocate_combination(int channels, int order) {
	int result = 0;
	combination_t *combination = NULL;
	combination = (combination_t *)malloc(sizeof(combination_t));
	int i;

	if ( combination == NULL ) {
		result = -1;
	} else {
		combination->channels = channels;
		combination->order = order;
		combination->digits = (int *)malloc(sizeof(int)*combination->order);
		if ( combination->digits == NULL ) {
			result = -1;
		} else {
			for ( i = 0; i < order; i++ ) {
				combination->digits[i] = 0;
			}
		}
	}

	if ( result ) {
		free_combination(&combination);
	}

	return(combination);
}

void free_combination(combination_t **combination) {
	if ( *combination != NULL ) {
		free((*combination)->digits);
		free(*combination);
	}
}

int next_combination(combination_t *combination) {
	int i;
	for ( i = (combination->order-1); i >= 0; i-- ) {
		combination->digits[i] = (combination->digits[i] + 1) 
									% combination->channels;

		if ( combination->digits[i] != 0 ) {
			/* No overflow */
			i = 0;
		} else if ( i == 0 ) {
			/* Overflow on leading digit, so we have reached 
			 * a total overflow.
			 */
			return(-1);
		}
	}

	return(0);
}

int get_combination_index(combination_t *combination) {
	int result = 0;
	int i;
	int base_value = 1;

	for ( i = combination->order-1; i >= 0; i-- ) {
		/*debug("combination index %d: %d * %d\n", i, base_value,
				combination->digits[i]); */
		result += base_value*combination->digits[i];
		base_value *= combination->channels;
	}

	return(result);
}

void print_combination(FILE *stream_out, combination_t *combination) {
	int i;

	for ( i = 0; i < combination->order; i++ ) {
		fprintf(stream_out, "%d,", combination->digits[i]);
	}
	fprintf(stream_out, "\b\n");
}

/*
 *
 * Offsets follow a form similar to that of combinations, but they are 
 * strictly increasing and can have limits not tied to the number of channels,
 * due to the fact that they represent the displacement of an index from the
 * start of a queue.
 *
 */
offsets_t *allocate_offsets(int order) {
	int result = 0;
	offsets_t *offsets;
	offsets = (offsets_t *)malloc(sizeof(offsets_t));
	if ( offsets == NULL ) {
		result = -1;
	} else {
		offsets->limit = order;
		offsets->order = order;
		offsets->offsets = (int *)malloc(sizeof(int)*offsets->order);
		if ( offsets->offsets == NULL ) {
			result = -1;
		}
	}

	if ( result ) {
		free_offsets(&offsets);
	}
		
	return(offsets);
}

void free_offsets(offsets_t **offsets) {
	if ( *offsets != NULL ) {
		if ( (*offsets)->offsets != NULL ) {
			free((*offsets)->offsets);
		}
		free(*offsets);
	}
}

int next_offsets(offsets_t *offsets) {
	int i;
	int leading_digit;

	leading_digit = offsets->offsets[1];

	for ( i = offsets->order-1; i > 0; i-- ) { 
		offsets->offsets[i] += 1;
		if ( offsets->offsets[i] <= offsets->limit ) {
			/* No overflow */
			i = 0;
		} else {
			offsets->offsets[i] = 0;
		}
	}

	if ( offsets->offsets[1] == 0 ) {
		offsets->offsets[1] = leading_digit+1;
	}

	for ( i = 1; i < offsets->order; i++ ) {
		if ( offsets->offsets[i] == 0 ) {
			offsets->offsets[i] = offsets->offsets[i-1] + 1;
		} else {
			/* nothing, because we already incremented. */
			;
		}

		if ( offsets->offsets[i] > offsets->limit ) {
			return(-1);
		}
	}

	return(0);
}

void init_offsets(offsets_t *offsets) {
	int i;
	for ( i = 0; i < offsets->order; i++ ) {
		offsets->offsets[i] = i;
	}
	/* do this to be able to increment on the first go */
	offsets->offsets[offsets->order-1] -= 1; 
}

void print_offsets(offsets_t *offsets) {
	int i;
	
	printf("(");
	for ( i = 0; i < offsets->order; i++ ) {
		printf("%2d,", offsets->offsets[i]);
	}
	printf("\b)\n");
}

/*
 * 
 * Functions to produce all permutations of a range (0, 1, ... n-1)
 *
 */
permutations_t *make_permutations(int order, int latter_only) {
	permutations_t *permutations;
	combination_t *permutation;
	int done = 0;
	int permutation_index = 0;
	int i;

	permutations = allocate_permutations(order, latter_only);
	permutation = allocate_combination(order, order);
	
	if ( permutations == NULL || permutation == NULL ) { 
		error("Could not allocate permutations.");
	} else {
		debug("Finding permutations.\n");
		if ( ! is_permutation(permutation) ) {
			done = next_permutation(permutation);
		}

		while ( ! done ) {
			debug("Working on permutation %d of %d.\n", permutation_index,
					permutations->n_permutations);
			if ( verbose ) {
				print_combination(stderr, permutation);
			}
				
			if ( latter_only && permutation->digits[0] != 0 ) {
				/* Here, we only want to iterate over permutations of the
				 * last n-1 digits, so if the first digit gets incremented
				 * we have gone too far.
				 */
				done = 1;
			} else {
				/* We have found a permutation to work with, so add it to the
				 * collection.
				 */
				for ( i = 0; i < order; i++ ) {
					permutations->permutations[permutation_index][i] = 
							permutation->digits[i];
				}
	
				done = next_permutation(permutation);
				permutation_index++;
			}
		}
	}

	free_combination(&permutation);

	return(permutations);
}

permutations_t *allocate_permutations(int order, int latter_only) {
	permutations_t *permutations = NULL;
	int result = 0;
	int i;

	permutations = (permutations_t *)malloc(sizeof(permutations_t));
	if ( permutations == NULL ) {
		result = -1;
	} else {
		permutations->length = order;
		permutations->latter_only = latter_only;

		if ( latter_only ) {
			permutations->n_permutations = factorial(order - 1);
		} else {
			permutations->n_permutations = factorial(order);
		}

		permutations->permutations = (int **)malloc(sizeof(int *)*
				permutations->n_permutations);
		
		if ( permutations->permutations == NULL ) {
			result = -1;
		} else {
			for ( i = 0; i < permutations->n_permutations && ! result; i++ ) {
				permutations->permutations[i] = (int *)malloc(sizeof(int)*
						order);
				
				if ( permutations->permutations[i] == NULL ) {
					result = -1;
				}
			}
		}
	}

	if ( result ) { 
		free_permutations(&permutations);
		permutations = NULL;
	}

	return(permutations);
}

void free_permutations(permutations_t **permutations) {
	int i;
	
	if ( *permutations != NULL ) {
		for ( i = 0; i < (*permutations)->n_permutations; i++ ) {
			free((*permutations)->permutations[i]);
		}
		free((*permutations)->permutations);
		free((*permutations));
	}
}

int next_permutation(combination_t *permutation) {
	int result = 0;

	while ( ! (result = next_combination(permutation)) ) {
		if ( is_permutation(permutation) ) {
			break;
		}
	}

	return(result);
}

int is_permutation(combination_t *permutation) {
	/* Check that no digit is repeated in the permutation.
	 */
	int i, j;

	for ( i = 0; i < permutation->order; i++ ) {
		for ( j = i+1; j < permutation->order; j++ ) {
			if ( permutation->digits[i] == permutation->digits[j] ) {
				return(0);
			}
		}
	}

	return(1);
}
