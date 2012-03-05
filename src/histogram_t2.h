#ifndef HISTOGRAM_T2_H_
#define HISTOGRAM_T2_H_

#include <stdio.h>
#include "t2.h"
#include "histogram.h"

typedef struct {
	unsigned int ref_channel;
	t2_t *records;
} t2_correlated_t;

typedef struct {
	unsigned int *channels;
	unsigned int *counts;
} t2_histogram_t;

typedef struct {
	int n_histograms;
	limits_t time_limits;
	int order;
	int bins_per_histogram;
	t2_histogram_t *histograms;
} t2_histograms_t;

t2_histograms_t *allocate_t2_histograms(options_t *options);
void free_t2_histograms(t2_histograms_t **histograms);
void print_t2_histograms(FILE *out_stream, t2_histograms_t *histograms);

t2_correlated_t *allocate_t2_correlated(options_t *options);
void free_t2_correlated(t2_correlated_t **record);
int next_t2_correlated(FILE *in_stream, t2_correlated_t *record, 
		options_t *options);

int histogram_t2(FILE *in_stream, FILE *out_stream, options_t *options);

#endif
