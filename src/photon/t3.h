#ifndef T3_H_
#define T3_H_

#include <stdio.h>
#include "photon.h"
/*#include "correlate_photon.h"
# include "histogram_photon.h"*/

typedef struct {
	unsigned int channel;
	long long pulse;
	long long time;
} t3_t;

typedef int (*t3_next_t)(FILE *, t3_t *);
typedef int (*t3_print_t)(FILE *, t3_t const *);

int t3_fscanf(FILE *stream_in, t3_t *t3);
int t3_fprintf(FILE *stream_out, t3_t const *t3);

int t3_compare(void const *a, void const *b);
int t3_echo(FILE *stream_in, FILE *stream_out);

/*void t3_correlate(correlation_t *correlation);
int t3_correlation_fscanf(FILE *stream_in, correlation_t *correlation);
int t3_correlation_fprintf(FILE *stream_out, correlation_t const *correlation);

int t3_under_max_distance(void const *correlator);
int t3_over_min_distance(void const *correlator);

int t3_correlation_build_channels(correlation_t const *correlation,
		combination_t *channels_vector);
int t3_correlation_build_values(correlation_t const *correlation,
		values_vector_t *values_vector);*/

#endif
