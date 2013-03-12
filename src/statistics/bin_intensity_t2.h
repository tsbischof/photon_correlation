#ifndef BIN_INTENSITY_T2_H_
#define BIN_INTENSITY_T2_H_

#include <stdio.h>
#include "histogram_gn.h"
#include "t2.h"
#include "bin_intensity.h"

typedef struct {
	int bins;
	int channels;
	edges_t *edges;
	int64_t **bin_count;
} t2_bin_counts_t;

int bin_intensity_t2(FILE *stream_in, FILE *stream_out, options_t *options);

t2_bin_counts_t *allocate_t2_bin_counts(options_t *options);
void free_t2_bin_counts(t2_bin_counts_t **bin_counts);
void print_t2_bin_counts(FILE *stream_out, t2_bin_counts_t *bin_counts,
		options_t *options);

void t2_bin_counts_increment(t2_bin_counts_t *bin_counts,
		t2_t *record, t2_t *front, t2_t *back, options_t *options);

#endif
