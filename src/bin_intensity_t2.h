#ifndef BIN_INTENSITY_T2_H_
#define BIN_INTENSITY_T2_H_

#include <stdio.h>
#include "histogram_gn.h"
#include "t2.h"
#include "bin_intensity.h"

typedef struct {
	int length;
	long long int left_index;
	long long int right_index;
	t2_t *queue;
} counts_t2_queue_t;

typedef struct {
	int channels;
	edges_t *bin_edges;
	long long int **counts;
	long long int **bin_time_limits;
} counts_t2_t;

void usage(void);

#endif

int bin_intensity_t2(FILE *in_stream, FILE *out_stream, options_t *options);
counts_t2_t *allocate_counts_t2(int channels, limits_t *time_limits);
void init_counts_t2(counts_t2_t *counts);
void free_counts_t2(counts_t2_t **counts);
int increment_counts_t2(counts_t2_t *counts, int channel);
void print_counts_t2(FILE *out_stream, counts_t2_t *counts);
int next_counts_t2_queue(FILE *in_stream, t2_t *queue, options_t *options);
void print_counts_t2(FILE *out_stream, counts_t2_t *counts);
int count_t2(t2_t *queue, options_t *options);

#endif
