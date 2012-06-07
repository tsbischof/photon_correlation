#ifndef BIN_INTENSITY_T2_H_
#define BIN_INTENSITY_T2_H_

#include <stdio.h>
#include "histogram_gn.h"
#include "t2.h"
#include "bin_intensity.h"

typedef struct {
	int bins;
	int channels;
	edges_t *bin_edges;
	bin_counts_t *bin_counts;
} t2_counts_t;

int bin_intensity_t2(FILE *in_stream, FILE *out_stream, options_t *options);
t2_counts_t *allocate_t2_counts(int channels, options_t *options);
void init_t2_counts(t2_counts_t *counts, options_t *options);
void free_t2_counts(t2_counts_t **counts);

int next_t2_counts_queue(FILE *in_stream, t2_queue_t *queue, 
			t2_counts_t *counts, options_t *options);

void print_t2_counts(FILE *out_stream, t2_counts_t *counts);

int count_t2(t2_queue_t *queue, options_t *options);

#endif
