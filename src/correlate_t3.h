#ifndef CORRELATE_T3_H_
#define CORRELATE_T3_H_

#include <stdio.h>
#include "combinations.h"
#include "t3.h"
#include "options.h"

typedef struct {
	long long int pulse;
	long long int time;
} t3_delay_t;

typedef struct {
	int order;
	int *channels;
	t3_delay_t *delays;
} t3_correlation_t;

int next_t3_queue_correlate(FILE *in_stream, 
		t3_queue_t *queue, options_t *options);

int valid_distance_t3(t3_t *left, t3_t *right, options_t *options);
int under_max_distance_t3(t3_t *left, t3_t *right, options_t *options);
int over_min_distance_t3(t3_t *left, t3_t *right, options_t *options);

int correlate_t3(FILE *in_stream, FILE *out_stream, options_t *options);
int correlate_t3_block(FILE *out_stream, long long int *record_number,
		t3_queue_t *queue,
		permutations_t *permutations,
		offsets_t *offsets, t3_t *correlation_block, 
		t3_correlation_t *correlation, options_t *options);

t3_correlation_t *allocate_t3_correlation(options_t *options);
void free_t3_correlation(t3_correlation_t **correlation);
void print_t3_correlation(FILE *out_stream, t3_correlation_t *correlation,
		options_t *options);

#endif
