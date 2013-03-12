#ifndef CORRELATION_PHOTON_H_
#define CORRELATION_PHOTON_H_

#include <stdio.h>
#include "../options.h"
#include "correlation.h"
#include "../combinatorics/combinations.h"

int correlate_photon(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);
int correlations_echo(FILE *stream_in, FILE *stream_out,
		pc_options_t const *options);

void t2_correlate(correlation_t *correlation);
int t2_correlation_fscanf(FILE *stream_in, correlation_t *correlation);
int t2_correlation_fprintf(FILE *stream_out, correlation_t const *correlation);

int t2_under_max_distance(void const *correlator);
int t2_over_min_distance(void const *correlator);

void t3_correlate(correlation_t *correlation);
int t3_correlation_fscanf(FILE *stream_in, correlation_t *correlation);
int t3_correlation_fprintf(FILE *stream_out, correlation_t const *correlation);

int t3_under_max_distance(void const *correlator);
int t3_over_min_distance(void const *correlator);

#endif

