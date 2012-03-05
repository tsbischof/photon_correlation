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

int channel_compare(const void *a, const void *b) {
	/* To preserve the order, ties go to the first entry. */
	int channel_0 = (*(channel_t *)a).channel;
	int channel_1 = (*(channel_t *)b).channel;

	if ( channel_0 <= channel_1 ) {
		return(-1);
	} else {
		return(1);
	}
}

/*
 *
 * Functions to build and loop through the combinations.
 *
 */
combinations_t *make_combinations(int channels, int order) {
	/* We want to produce all combinations of length order, with index 
	 * choices 0...(channels-1). We will produce these in order, as though
	 * the elements are digits of a base (channels) number. This allows us to 
	 * build a lookup table of the indices to call in order.
	 */
	int i;
	int result = 0;
	int combination_index = 0;
	int done = 0;
	channel_t *channels_array;
	combinations_t *combinations;
	combination_t *combination; 

	/* Allocate memory for the combinations. */
	debug("Allocating memory for the channel combination lookup table.\n");
	combinations = allocate_combinations(channels, order);
	combination = allocate_combination(channels, order);
	channels_array = (channel_t *)malloc(sizeof(channel_t)*order);

	if ( combinations == NULL || combination == NULL 
			|| channels_array == NULL ) {
		result = -1;
	}

	/* Start generating the combinations. */
	if ( result ) {
		error("Could not allocate memory for the combinations.\n");
		free_combinations(&combinations);
	} else {
		debug("Creating the channel combination lookup table.\n");
		/* Everything worked out fine, do the calculation. */
		while ( ! done ) {
			/* Create an array with the channels and indices. We will then
			 * sort by the channels.
			 */
#if PRINT_TUPLES
			printf("-------- %8d --------\n", combination_index);
			printf("(");
#endif
			for ( i = 0; i < order; i++ ) {
				channels_array[i].channel = combination->digits[i];
				channels_array[i].index = i;
#if PRINT_TUPLES
				printf("(%d, %d) ", 
						channels_array[i].channel, 
						channels_array[i].index);
#endif
			}
#if PRINT_TUPLES
			printf("\b) --> \n");
#endif

			/* Now that we have the (channel, index) pairs, sort by the
			 * channel.
			 */
			debug("Checking whether the entries are sorted.\n");
			combinations->sorted[combination_index] = 1;

			for ( i = 0; i < order-1; i++ ) {
				if ( channels_array[i].channel > channels_array[i+1].channel) {
					combinations->sorted[combination_index] = 0;
				}
			}

			if ( ! combinations->sorted[combination_index] ) {
				debug("Sorting the entries.\n");
				qsort(&channels_array[0], 
						order, sizeof(channel_t), channel_compare);
			}

#if PRINT_TUPLES
			printf("(");
			for ( i = 0; i < order; i++ ) {
				printf("(%d, %d) ", 
						channels_array[i].channel, 
						channels_array[i].index);
			}
			printf("\b) --> \n");
#endif

			/* Generate the final lookup table. 
			 */
			debug("Generating the indices table.\n");
			for ( i = 0; i < order; i++ ) {
				combinations->indices[combination_index][i] = 
						channels_array[i].index;
			}

#if PRINT_TUPLES
			printf("(");
			for ( i = 0; i < order; i++ ) {
				printf("%d, ", 
						combinations->indices[combination_index][i]);
			}
			printf("\b)\n"); 
#endif

			/* Get the next combination, now that we are done with this one. */
			combination_index++;
			done = next_combination(combination);
		}
	}

	free(channels_array);
	free(combination);

	return(combinations);
}

combinations_t *allocate_combinations(int channels, int order) {
	combinations_t *combinations;
	int result = 0;
	int i;

	combinations = (combinations_t *)malloc(sizeof(combinations_t));
	if ( combinations == NULL ) {
		result = -1;
	} else {
		combinations->channels = channels;
		combinations->order = order;
		combinations->n_combinations = n_combinations(channels, order);

		if ( combinations->n_combinations == 0 ) {
			result = -1;
		} else {
			combinations->indices = (int **)malloc(sizeof(int *)
												*combinations->n_combinations);
			combinations->sorted = (int *)malloc(sizeof(int)
												*combinations->n_combinations);
	
			if ( combinations->indices == NULL ) {
				result = -1;
			} else {
				for ( i = 0; i < combinations->n_combinations; i++ ) {
					combinations->indices[i] = (int *)malloc(sizeof(int)
												*combinations->order);
					if ( combinations->indices[i] == NULL ) {
						i = combinations->n_combinations;
						result = -1;
					}
				}
			}
		}
	}

	if ( result ) {
		free_combinations(&combinations);
		combinations = NULL;
	}

	return(combinations);
}

void free_combinations(combinations_t **combinations) {
	int i;
	
	if ( *combinations != NULL ) {
		if ( (*combinations)->indices != NULL ) {
			for ( i = 0; i < (*combinations)->n_combinations; i++ ) {
				free((*combinations)->indices[i]);
			}
			free((*combinations)->indices);
		}

		free((*combinations)->sorted);

		free(*combinations);
	}
}


/*
 *
 * Functions to deal with individual combinations.
 *
 */
combination_t *allocate_combination(int channels, int order) {
	int result = 0;
	combination_t *combination;
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
		if ( (*combination)->digits != NULL ) {
			free((*combination)->digits);
		}
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
		debug("combination index %d: %d * %d\n", i, base_value,
				combination->digits[i]);
		result += base_value*combination->digits[i];
		base_value *= combination->channels;
	}

	return(result);
}

void print_combinations(combinations_t *combinations) {
	int i;
	int j;

	combination_t *combination;

	combination = allocate_combination(combinations->channels,
										combinations->order);

	if ( combination == NULL ) {
		error("Could not allocate combination memory.\n");
	} else {
		for ( i = 0; i < combinations->n_combinations; i++ ) {
			printf("-------- %8d --------\n", i);
			printf("(");
			for ( j = 0; j < combinations->order; j++ ) {
				printf("%2d,", combination->digits[j]);
			} 
			printf("\b)\n(");
			for ( j = 0; j < combinations->order; j++ ) {
				printf("%2d,", combinations->indices[i][j]);
			} 
			printf("\b)\n");
			next_combination(combination);
		}
	}

	free_combination(&combination);
}

/*
 *
 * Offsets follow a form similar to that of combinations, but they are 
 * strictly increasing and can have limits not tied to the number of channels,
 * due to the fact that they represent the displacement of an index from the
 * start of a queue.
 *
 */
offsets_t *allocate_offsets(int channels, int order) {
	int result = 0;
	offsets_t *offsets;
	offsets = (offsets_t *)malloc(sizeof(offsets));
	if ( offsets == NULL ) {
		result = -1;
	} else {
		offsets->limit = channels;
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
		free((*offsets));
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
