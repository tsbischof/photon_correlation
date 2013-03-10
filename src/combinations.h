#ifndef COMBINATIONS_H_
#define COMBINATIONS_H_

#include <stdio.h>
#include "options.h"

unsigned int pow_int(unsigned int const base, unsigned int const exponent);
unsigned int n_combinations(unsigned int const n, unsigned int const m);
unsigned int n_permutations(unsigned int const n);
unsigned int factorial(unsigned int const n);

typedef struct {
	unsigned int length;
	unsigned int limit;

	int yielded;
	unsigned int *values;
} combination_t;

combination_t *combination_alloc(unsigned int const length,
		unsigned int const limit);
void combination_init(combination_t *combination);
void combination_free(combination_t **combination);
int combination_next(combination_t *combination);
unsigned int combination_index(combination_t const *combination);
int combination_fprintf(FILE *stream_out, combination_t const *combination);

typedef struct {
	unsigned int length;
	unsigned int limit;
	int yielded;

	combination_t *current_index_offsets;
} index_offsets_t;

index_offsets_t *index_offsets_alloc(unsigned int const length);
void index_offsets_init(index_offsets_t *index_offsets, 
		unsigned int const limit);
int index_offsets_next(index_offsets_t *index_offsets);
void index_offsets_free(index_offsets_t **index_offsets);

int combinations_dispatch(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif 
