#ifndef COMBINATIONS_H_
#define COMBINATIONS_H_

#include <stdio.h>

typedef struct {
	int channel;
	int index;
} channel_t;

typedef struct {
	int n_permutations;
	int length;
	int latter_only;
	int **permutations;
} permutations_t;


typedef struct {
	int channels;
	int order;
	int *digits;
} combination_t;

typedef struct {
	int limit;
	int order;
	int *offsets;
} offsets_t;

int pow_int(int channels, int order);
int n_combinations(int channels, int order);
int n_permutations(int order);
int factorial(int order);

combination_t *allocate_combination(int channels, int order);
void free_combination(combination_t **combination);
int next_combination(combination_t *combination);
int get_combination_index(combination_t *combination);
void print_combination(FILE *stream_out, combination_t *combination);

offsets_t *allocate_offsets(int order);
void free_offsets(offsets_t **offsets);
int next_offsets(offsets_t *offsets);
void init_offsets(offsets_t *offsets);
void print_offsets(offsets_t *offsets);

permutations_t *make_permutations(int order, int latter_only);
permutations_t *allocate_permutations(int order, int latter_only);
void free_permutations(permutations_t **permutations);
int next_permutation(combination_t *permutation);
int is_permutation(combination_t *permutation);

#endif 
