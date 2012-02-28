#ifndef COMBINATIONS_H_
#define COMBINATIONS_H_

typedef struct {
	int channel;
	int index;
} channel_t;

typedef struct {
	int channels;
	int order;
	int n_combinations;
	int **indices;
} combinations_t;

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

int n_combinations(int channels, int order);
int channel_compare(const void *a, const void *b);

combinations_t *make_combinations(int channels, int order);
combinations_t *allocate_combinations(int channels, int order);
void free_combinations(combinations_t **combinations);
void print_combinations(combinations_t *combinations);

combination_t *allocate_combination(int channels, int order);
void free_combination(combination_t **combination);
int next_combination(combination_t *combination);
int get_combination_index(combination_t *combination);


offsets_t *allocate_offsets(int channels, int order);
void free_offsets(offsets_t **offsets);
int next_offsets(offsets_t *offsets);
void init_offsets(offsets_t *offsets);
void print_offsets(offsets_t *offsets);

#endif 
