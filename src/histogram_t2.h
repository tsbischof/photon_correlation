#ifndef HISTOGRAM_T2_H_
#define HISTOGRAM_T2_H_

#include <stdio.h>
#include "t2.h"
#include "histogram_gn.h"
#include "combinations.h"
#include "options.h"

typedef struct {
	unsigned int ref_channel;
	t2_t *records;
} t2_correlated_t;

typedef struct {
	int n_histograms;
	edges_t **edges;
	int channels;
	int order;
	combination_t *combination;
	long long int *current_values;
	gn_histogram_t **histograms;
} t2_histograms_t;

t2_correlated_t *allocate_t2_correlated(options_t *options);
void free_t2_correlated(t2_correlated_t **record);
int next_t2_correlated(FILE *in_stream, t2_correlated_t *record, 
		options_t *options);

t2_histograms_t *make_t2_histograms(options_t *options);
void free_t2_histograms(t2_histograms_t **histograms);
int t2_histograms_increment(t2_histograms_t *histograms, 
		t2_correlated_t *record);
void print_t2_histograms(FILE *out_stream, t2_histograms_t *histograms);

int histogram_t2(FILE *in_stream, FILE *out_stream, options_t *options);

#endif
