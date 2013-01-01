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

int intensity_dispatch(FILE *stream_in, FILE *stream_out, options_t *options);

counts_t *allocate_counts(int channels);
void init_counts(counts_t *counts);
void free_counts(counts_t **counts);
int increment_counts(counts_t *counts, int channel);
void print_counts(FILE *stream_out, counts_t *counts, options_t *options);
int next_counts(FILE *stream_in, counts_t *counts, options_t *options);

#endif
