#ifndef INTENSITY_UTIL_H_
#define INTENSITY_UTIL_H_

#include <stdio.h>
#include "limits.h"
#include "options.h"

typedef struct {
	int channels;
	window_t window;
	int64_t *counts;
} counts_t;

int intensity_dispatch(FILE *in_stream, FILE *out_stream, options_t *options);

counts_t *allocate_counts(int channels);
void init_counts(counts_t *counts);
void free_counts(counts_t **counts);
int increment_counts(counts_t *counts, int channel);
void print_counts(FILE *out_stream, counts_t *counts, options_t *options);
int next_counts(FILE *in_stream, counts_t *counts, options_t *options);

#endif
