#ifndef HISTOGRAM_T3_H_
#define HISTOGRAM_T3_H_

#include <stdio.h>
#include "t3.h"
#include "histogram_gn.h"
#include "correlate_t3.h"
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
} t3_histograms_t;

t3_histograms_t *make_t3_histograms(options_t *options);
void free_t3_histograms(t3_histograms_t **histograms);
int t3_histograms_increment(t3_histograms_t *histograms, 
		t3_correlation_t *record);
void print_t3_histograms(FILE *stream_out, t3_histograms_t *histograms,
		options_t *options);

int histogram_t3(FILE *stream_in, FILE *stream_out, options_t *options);
int histogram_t3_g1(FILE *stream_in, FILE *stream_out, options_t *options);
int histogram_t3_gn(FILE *stream_in, FILE *stream_out, options_t *options);

#endif
