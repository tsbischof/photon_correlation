#ifndef CORRELATE_T3_H_
#define CORRELATE_T3_H_

#include <stdio.h>
#include "correlate.h"
#include "combinations.h"
#include "t3.h"

typedef struct {
	int length;
	long long int left_index;
	long long int right_index;
	t3_t *queue;
} t3_queue_t;

t3_queue_t *allocate_t3_queue(options_t *options);
void free_t3_queue(t3_queue_t **queue);
t3_t get_queue_item_t3(t3_queue_t *queue, int index);
int next_t3_queue(FILE *in_stream, t3_queue_t *queue, options_t *options);

int valid_distance_t3(t3_t *left, t3_t *right, options_t *options);

int correlate_t3(FILE *in_stream, FILE *out_stream, options_t *options);
int correlate_t3_block(FILE *out_stream, t3_queue_t *queue,
		combinations_t *combinations, combination_t *combination,
		offsets_t *offsets, t3_t *correlation_block, options_t *options);

#endif
