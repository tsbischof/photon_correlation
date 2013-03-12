#ifndef HISTOGRAM_PHOTON_H_
#define HISTOGRAM_PHOTON_H_

#include <stdio.h>
#include "../options.h"
#include "values_vector.h"
#include "../combinatorics/combinations.h"
#include "../correlation/correlation.h"

int t2_correlation_build_channels(correlation_t const *correlation,
		combination_t *channels_vector);
int t2_correlation_build_values(correlation_t const *correlation,
		values_vector_t *values_vector);

int t3_correlation_build_channels(correlation_t const *correlation,
		combination_t *channels_vector);
int t3_correlation_build_values(correlation_t const *correlation,
		values_vector_t *values_vector);

int histogram_photon(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif 
