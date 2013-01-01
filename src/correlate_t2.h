#ifndef CORRELATE_T2_H_
#define CORRELATE_T2_H_

#include <stdio.h>

#include "combinations.h"
#include "t2.h"
#include "options.h"
#include "types.h"

typedef struct {
	int order;
	t2_t *records;
} t2_correlation_t;

int next_t2_queue_correlate(FILE *stream_in, 
		t2_queue_t *queue, options_t *options);

int valid_distance_t2(t2_t *left, t2_t *right, options_t *options);
int under_max_distance_t2(t2_t *left, t2_t *right, options_t *options);
int over_min_distance_t2(t2_t *left, t2_t *right, options_t *options);

int correlate_t2(FILE *stream_in, FILE *stream_out, options_t *options);
int correlate_t2_block(FILE *stream_out, int64_t *record_number,
		t2_queue_t *queue,
		permutations_t *permutations,
		offsets_t *offsets, t2_t *correlation_block, 
		t2_correlation_t *correlation, options_t *options);

t2_correlation_t *allocate_t2_correlation(options_t *options);
void free_t2_correlation(t2_correlation_t **correlation);
int next_t2_correlation(FILE *stream_in, t2_correlation_t *correlation,
		options_t *options);
void print_t2_correlation(FILE *stream_out, t2_correlation_t *correlation, 
		int print_newline, options_t *options);

int correlate_t2_start_stop(FILE *stream_in, FILE *stream_out,
		options_t *options);

#endif
