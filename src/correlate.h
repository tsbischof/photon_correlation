#ifndef CORRELATE_H_
#define CORRELATE_H_

#include <stdio.h>
#include "options.h"
#include "photon.h"
#include "combinations.h"
#include "permutations.h"

int correlate_dispatch(FILE *stream_in, FILE *stream_out, 
		options_t const *options);

typedef struct {
	int mode;
	size_t photon_size;
	unsigned int order;
	void *photons;
} correlation_t;

typedef void (*correlate_t)(void *correlation);
typedef int (*correlation_next_t)(FILE *stream_out, 
		correlation_t *correlation);
typedef int (*correlation_print_t)(FILE *stream_out, 
		correlation_t const *correlation);

correlation_t *correlation_alloc(int const mode, unsigned int const order);
void correlation_init(correlation_t *correlation);
void correlation_free(correlation_t **correlation);

typedef struct {
	int mode;
	unsigned int order;

	photon_queue_t *photon_queue;

	combination_t *photon_combination;
	permutations_t *photon_permutations;

	correlation_t *current_correlation;

	correlate_t correlate;
	correlation_print_t correlation_print;
} correlator_t;

correlator_t *correlator_alloc(options_t const *options);
int correlator_init(correlator_t *correlator, photon_stream_t *photon_stream,
		options_t const *options);
int correlator_next(correlator_t *correlator);
void correlator_free(correlator_t **correlator);

int correlate_photon(FILE *stream_in, FILE *stream_out, 
		options_t const *options);

#endif

