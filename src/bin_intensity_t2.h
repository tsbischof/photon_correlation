#ifndef BIN_INTENSITY_T2_H_
#define BIN_INTENSITY_T2_H_

#include <stdio.h>
#include "histogram_gn.h"
#include "t2.h"
#include "bin_intensity.h"

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
