#ifndef INTENSITY_H_
#define INTENSITY_H_

#include <stdio.h>

typedef struct {
	char *in_filename;
	char *out_filename;
	char *mode_string;
	int mode;
	long long bin_width;
	int channels;
} options_t;

typedef struct {
	int channels;
	long long int *counts;
} counts_t;

void usage(void);
int intensity_t2(FILE *in_stream, FILE *out_stream, options_t *options);
int intensity_t3(FILE *in_stream, FILE *out_stream, options_t *options);

counts_t *allocate_counts(int channels);
void init_counts(counts_t *counts);
void free_counts(counts_t **counts);
int increment_counts(counts_t *counts, int channel);
void print_counts(FILE *out_stream, long long int time, counts_t *counts);

#endif
