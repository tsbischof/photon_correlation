#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include <stdio.h>

#define SCALE_UNKNOWN -1
#define SCALE_LINEAR 1
#define SCALE_LOG 2

typedef struct {
	long long int lower;
	unsigned int bins;
	long long int upper;
} limits_t;

typedef struct {
	char *in_filename;
	char *out_filename;
	char *mode_string;
	int mode;
	char *time_string;
	limits_t time_limits;
	char *pulse_string;
	limits_t pulse_limits;
	int channels;
	int order;
	char *time_scale_string;
	int time_scale;
	char *pulse_scale_string;
	int pulse_scale;
} options_t;

typedef struct {
	unsigned int *counts;
} g1_histogram_t;

typedef struct {
	int n_histograms;
	limits_t limits;
	long long int *bin_edges;
	g1_histogram_t *histograms;
} g1_histograms_t;

void usage(void);
int str_to_limits(char *str, limits_t *limits);
int scale_parse(char *str, int *scale);

g1_histograms_t *allocate_g1_histograms(limits_t *limits, int channels);
void free_g1_histograms(g1_histograms_t **g1_histogram);
void print_g1_histograms(FILE *out_stream, g1_histograms_t *histograms);
int increment_g1_histograms(g1_histograms_t *histograms, 
		unsigned int channel, long long int value);

#endif
