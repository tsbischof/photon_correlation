#ifndef CORRELATE_T2_H_
#define CORRELATE_T2_H_

#include <stdio.h>
#include "correlate.h"
#include "combinations.h"
#include "t2.h"

typedef struct {
	int length;
	long long int left_index;
	long long int right_index;
	t2_t *queue;
} t2_queue_t;

t2_queue_t *allocate_t2_queue(options_t *options);
void free_t2_queue(t2_queue_t **queue);
t2_t get_queue_item_t2(t2_queue_t *queue, int index);
int next_t2_queue(FILE *in_stream, t2_queue_t *queue, options_t *options);

int valid_distance_t2(t2_t *left, t2_t *right, options_t *options);

int correlate_t2(FILE *in_stream, FILE *out_stream, options_t *options);
int correlate_t2_block(FILE *out_stream, t2_queue_t *queue,
		combinations_t *combinations, combination_t *combination,
		offsets_t *offsets, t2_t *correlation_block, options_t *options);

#endif
