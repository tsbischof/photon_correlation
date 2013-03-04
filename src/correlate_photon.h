#ifndef CORRELATE_PHOTON_H_
#define CORRELATE_PHOTON_H_

#include <stdio.h>
#include "options.h"
#include "photon.h"
#include "combinations.h"
#include "permutations.h"

int correlate_photon(FILE *stream_in, FILE *stream_out, 
		options_t const *options);
int correlations_echo(FILE *stream_in, FILE *stream_out,
		options_t const *options);

typedef struct {
	int mode;
	size_t photon_size;
	unsigned int order;
	void *photons;
} correlation_t;

typedef void (*correlate_t)(correlation_t *correlation);
typedef int (*correlation_next_t)(FILE *stream_out, 
		correlation_t *correlation);
typedef int (*correlation_print_t)(FILE *stream_out, 
		correlation_t const *correlation);

correlation_t *correlation_alloc(int const mode, unsigned int const order);
void correlation_init(correlation_t *correlation);
int correlation_set_index(correlation_t *correlation,
		void const *photon,
		unsigned int const index);
void correlation_free(correlation_t **correlation);

typedef struct {
	int mode;
	unsigned int order;

	int in_block;
	int in_permutations;
	int eof;

	size_t photon_size;
	void *left;
	void *right;
	void *scratch;

	int64_t min_pulse_distance;
	int64_t max_pulse_distance;
	int64_t min_time_distance;
	int64_t max_time_distance;

	photon_stream_t *photon_stream;

	photon_queue_t *photon_queue;

	index_offsets_t *index_offsets;
	permutation_t *photon_permutation;

	correlation_t *current_correlation;

	correlate_t correlate;
	correlation_print_t correlation_print;

	int (*under_max_distance)(void const *correlator);
	int (*over_min_distance)(void const *correlator);
} correlator_t;

correlator_t *correlator_alloc(options_t const *options);
int correlator_init(correlator_t *correlator, photon_stream_t *photon_stream,
		options_t const *options);
int correlator_next(correlator_t *correlator);
void correlator_free(correlator_t **correlator);
int correlator_populate(correlator_t *correlator);
void correlator_block_init(correlator_t *correlator);
int correlator_yield_from_block(correlator_t *correlator);
int correlator_valid_distance(correlator_t *correlator);
int correlator_build_correlation(correlator_t *correlator);
void correlator_flush(correlator_t *correlator);

#endif

