#ifndef CORRELATOR_LOG_H_
#define CORRELATOR_LOG_H_

#include <stdio.h>
#include "../options.h"
#include "../photon/t2.h"
#include "../queue.h"
#include "correlation.h"

typedef struct {
	queue_t *photons;
	queue_t *pairs;

	int flushing;
	long long max_time_distance;

	correlation_t *correlation;
	correlation_print_t correlation_print;
	correlate_t correlate;

	t2_t left;
	t2_t right;

	unsigned long long a;
	unsigned long long x;
	unsigned long long c;
	unsigned long long m;
} correlator_log_t;

correlator_log_t *correlator_log_alloc(size_t const queue_size);
void correlator_log_init(correlator_log_t *correlator,
		long long const max_time_distance,
		unsigned int seed);
void correlator_log_free(correlator_log_t **correlator);
int correlator_log_push(correlator_log_t *correlator, t2_t *t2);
int correlator_log_next(correlator_log_t *correlator);
void correlator_log_flush(correlator_log_t *correlator);
int correlator_log_form_pairs(correlator_log_t *correlator);

int correlate_log(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
