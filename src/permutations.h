#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

#include <stdio.h>
#include "combinations.h"

typedef struct {
	int positive_only;
	unsigned int n_permutations;
	unsigned int length;
	int yielded;
	unsigned int current_index;

	combinations_t *scratch;
	combination_t **permutations;
	combination_t *current_permutation;
} permutations_t;

permutations_t *permutations_alloc(unsigned int const length, 
		int const positive_only);
void permutations_init(permutations_t *permutations);
int permutations_next(permutations_t *permutations);
void permutations_free(permutations_t **permutations);

int is_permutation(combination_t const *combination);
int is_positive_permutation(combination_t const *combination);

#endif
