#ifndef CORRELATE_T2_H_
#define CORRELATE_T2_H_

#include <stdio.h>
#include "combinations.h"
#include "t2.h"
#include "options.h"

typedef long long int t2_delay_t;

typedef struct {
	int order;
	unsigned int *channels;
	t2_delay_t *delays;
} t2_correlation_t;

int next_t2_queue(FILE *in_stream, t2_queue_t *queue, options_t *options);

int valid_distance_t2(t2_t *left, t2_t *right, options_t *options);
int under_max_distance_t2(t2_t *left, t2_t *right, options_t *options);
int over_min_distance_t2(t2_t *left, t2_t *right, options_t *options);

int correlate_t2(FILE *in_stream, FILE *out_stream, options_t *options);
int correlate_t2_block(FILE *out_stream, t2_queue_t *queue,
		permutations_t *permutations,
		offsets_t *offsets, t2_t *correlation_block, 
		t2_correlation_t *correlation, options_t *options);

t2_correlation_t *allocate_t2_correlation(options_t *options);
void free_t2_correlation(t2_correlation_t **correlation);
void print_t2_correlation(FILE *out_stream, t2_correlation_t *correlation, 
		options_t *options);

#endif
