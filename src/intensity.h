#ifndef INTENSITY_UTIL_H_
#define INTENSITY_UTIL_H_

#include <stdio.h>
#include "options.h"

typedef struct {
	int channels;
	long long int *counts;
} counts_t;

int intensity_dispatch(FILE *in_stream, FILE *out_stream, options_t *options);

counts_t *allocate_counts(int channels);
void init_counts(counts_t *counts);
void free_counts(counts_t **counts);
int increment_counts(counts_t *counts, int channel);
void print_counts(FILE *out_stream, long long int lower_time, 
		long long int upper_time, counts_t *counts);

#endif
