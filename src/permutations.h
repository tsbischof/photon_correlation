#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

#include <stdio.h>
#include "combinations.h"

typedef struct {
	int positive_only;
	unsigned int n_permutations;
	unsigned int length;
	unsigned int current_index;
	
	int populated;
	int yielded;

	combination_t *scratch;
	unsigned int **permutations;
	unsigned int *values;
} permutation_t;

permutation_t *permutation_alloc(unsigned int const length, 
		int const positive_only);
void permutation_init(permutation_t *permutation);
int permutation_next(permutation_t *permutation);
void permutation_free(permutation_t **permutation);
int permutation_fprintf(FILE *stream_out, permutation_t const *permutation);

int is_permutation(combination_t const *combination);
int is_positive_permutation(combination_t const *combination);

#endif
