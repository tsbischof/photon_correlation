#include <stdio.h>
#include <stdlib.h>

#include "../error.h"
#include "combinations.h"

/* Set this to 1 to show all of the tuples of (channel, index) and whatnot. 
 * This is only really necessary for a full-scale debug, and should not be 
 * shown for a simple --verbose run.
 */
#define PRINT_TUPLES 0


unsigned int pow_int(unsigned int const base, unsigned int const exponent) {
	unsigned int result = 1;
	int i;

	for ( i = 0; i < exponent; i++ ) {
		result *= base;
	}

	return(result);
}

unsigned long long powull(unsigned long long const base, 
		unsigned long long const exponent) {
	unsigned long long result = 1;
	int i;

	for ( i = 0; i < exponent; i++ ) {
		result *= base;
	}

	return(result);
}

unsigned int n_combinations(unsigned int const n, unsigned int const m) {
	/* The number of combinations will be channels^order, since we have
 	 * order many digits to populate with a channel (up to channels-1)
 	 */
	return(pow_int(n, m));
}

unsigned int n_permutations(unsigned int const n) { 
	return(factorial(n));
}

unsigned int factorial(unsigned int const n) {
	unsigned int result = 1;
	int i;

	for ( i = 1; i <= n; i++ ) {
		result *= i;
	}

	return(result);
}


combination_t *combination_alloc(unsigned int const length, 
		unsigned int const limit) {
	combination_t *combination = NULL;

	combination = (combination_t *)malloc(sizeof(combination_t));

	if ( combination == NULL ) {
		return(combination);
	}

	combination->length = length;
	combination->limit = limit;
	combination->values = (unsigned int *)malloc(sizeof(unsigned int)*length);

	if ( combination->values == NULL ) {
		combination_free(&combination);
		return(combination);
	}

	return(combination);
}

void combination_init(combination_t *combination) {
	int i;

	combination->yielded = 0;

	for ( i = 0; i < combination->length; i++ ) {
		combination->values[i] = 0;
	}
}

void combination_free(combination_t **combination) {
	if ( *combination != NULL ) {
		free((*combination)->values);
		free(*combination);
	}
}

unsigned int combination_index(combination_t const *combination) {
/* Treat the combination as a base-length integer. */
	unsigned int result = 0;
	int i;
	unsigned int base_value = 1;

	for ( i = combination->length-1; i >= 0; i-- ) {
		result += base_value*combination->values[i];
		base_value *= combination->limit;
	}

	return(result);
}

int combination_fprintf(FILE *stream_out, combination_t const *combination) {
	int i;

	for ( i = 0; i < combination->length; i++ ) {
		fprintf(stream_out, "%u", combination->values[i]);

		if ( i+1 < combination->length ) {
			fprintf(stream_out, ",");
		}
	}

	fprintf(stream_out, "\n");

	return( ferror(stream_out) ? PC_ERROR_IO : PC_SUCCESS );
}

int combination_next(combination_t *combination) {
/* Starting from the final digit, increment. Pass the overflow along,
 * until reaching the beginning. If all overflow, we have reached the end.
 */
	int i;

	if ( combination->yielded ) {
		debug("Yielded last value, produce the next.\n");
		for ( i = combination->length - 1; i >= 0; i-- ) {
			combination->values[i] = (combination->values[i] + 1) %
					combination->limit;

			if ( combination->values[i] != 0 ) {
				i = 0;
			} else if ( i == 0 ) {
				return(PC_COMBINATION_OVERFLOW);
			}
		}

		return(PC_SUCCESS);
	} else {
		debug("Not yet yielded, use this one.\n");
		combination->yielded = 1;
		return(PC_SUCCESS);
	}
}
