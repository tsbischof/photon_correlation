#ifndef INTENSITY_H_
#define INTENSITY_H_

#include <stdio.h>
#include "histogram_gn.h"

typedef struct {
	char *in_filename;
	char *out_filename;
	char *mode_string;
	int mode;
	long long bin_width;
	char time_string;
	limits_t time_limits;
	char pulse_string;
	limits_t pulse_limits;
	int channels;
} options_t;

typedef struct {
	int channels;
	edges_t *edges;
	long long int **counts;
} counts_t2_t;

void usage(void);
int bin_intensity_t2(FILE *in_stream, FILE *out_stream, options_t *options);

counts_t2_t *allocate_bin_counts_t2(int channels, limits_t *time_limits);
void init_counts_t2(counts_t2_t *counts);
void free_counts_t2(counts_t2_t **counts);
int increment_counts_t2(counts_t2_t *counts, int channel);
void print_counts_t2(FILE *out_stream, long long int lower_time, 
		long long int upper_time, counts_t2_t *counts);

#endif
