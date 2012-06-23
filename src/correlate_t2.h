#ifndef CORRELATE_T2_H_
#define CORRELATE_T2_H_

#include <stdio.h>
#include "correlate.h"
#include "combinations.h"
#include "t2.h"
#include "options.h"

int next_t2_queue(FILE *in_stream, t2_queue_t *queue, options_t *options);

int valid_distance_t2(t2_t *left, t2_t *right, options_t *options);
int under_max_distance_t2(t2_t *left, t2_t *right, options_t *options);
int over_min_distance_t2(t2_t *left, t2_t *right, options_t *options);

int correlate_t2(FILE *in_stream, FILE *out_stream, options_t *options);
int correlate_t2_block(FILE *out_stream, t2_queue_t *queue,
		permutations_t *permutations,
		offsets_t *offsets, t2_t *correlation_block, options_t *options);

#endif
