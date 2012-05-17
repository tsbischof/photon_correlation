#ifndef HISTOGRAM_T3_H_
#define HISTOGRAM_T3_H_

#include <stdio.h>
#include "histogram.h"
#include "t3.h"

typedef struct {
	unsigned int ref_channel;
	t3_t *records;
} t3_correlated_t;

typedef struct {
	int n_histograms;
	edges_t **edges;
	int channels;
	int order;
	combination_t *combination;
	long long int *current_values;
	gn_histogram_t **histograms;
} t3_histograms_t;

int histogram_t3(FILE *in_stream, FILE *out_stream, options_t *options);

int histogram_t3_g1(FILE *in_stream, FILE *out_stream, options_t *options);

int histogram_t3_gn(FILE *in_stream, FILE *out_stream, options_t *options);

t3_correlated_t *allocate_t3_correlated(options_t *options);
void free_t3_correlated(t3_correlated_t **record);
int next_t3_correlated(FILE *in_stream, t3_correlated_t *record, 
		options_t *options);

t3_histograms_t *make_t3_histograms(options_t *options);
void free_t3_histograms(t3_histograms_t **histograms);
int t3_histograms_increment(t3_histograms_t *histograms, 
		t3_correlated_t *record);
void print_t3_histograms(FILE *out_stream, t3_histograms_t *histograms);

#endif
