#ifndef BIN_INTENSITY_T3_H_
#define BIN_INTENSITY_T3_H_

#include <stdio.h>
#include "histogram_gn.h"
#include "t3.h"
#include "bin_intensity.h"

typedef struct {
	int bins;
	int channels;
	edges_t *edges;
	int64_t **bin_count;
} t3_bin_counts_t;

int bin_intensity_t3(FILE *in_stream, FILE *out_stream, options_t *options);

t3_bin_counts_t *allocate_t3_bin_counts(options_t *options);
void free_t3_bin_counts(t3_bin_counts_t **bin_counts);
void print_t3_bin_counts(FILE *out_stream, t3_bin_counts_t *bin_counts,
		options_t *options);

void t3_bin_counts_increment(t3_bin_counts_t *bin_counts,
		t3_t *record, t3_t *front, t3_t *back, options_t *options);

#endif
