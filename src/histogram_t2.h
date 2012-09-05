#ifndef HISTOGRAM_T2_H_
#define HISTOGRAM_T2_H_

#include <stdio.h>
#include "t2.h"
#include "histogram_gn.h"
#include "correlate_t2.h"
#include "combinations.h"
#include "options.h"
#include "types.h"

typedef struct {
	int n_histograms;
	edges_t **edges;
	int channels;
	int order;
	combination_t *combination;
	int64_t *current_values;
	gn_histogram_t **histograms;
} t2_histograms_t;

t2_histograms_t *make_t2_histograms(options_t *options);
void free_t2_histograms(t2_histograms_t **histograms);
int t2_histograms_increment(t2_histograms_t *histograms, 
		t2_correlation_t *record);
void print_t2_histograms(FILE *out_stream, t2_histograms_t *histograms);

int histogram_t2(FILE *in_stream, FILE *out_stream, options_t *options);

#endif
