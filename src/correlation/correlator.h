#ifndef CORRELATOR_H_
#define CORRELATOR_H_

#include "correlation.h"
#include "../photon/stream.h"
#include "../queue.h"
#include "../combinatorics/permutations.h"
#include "../combinatorics/index_offsets.h"

typedef struct {
	int mode;
	unsigned int order;

	int flushing;
	int in_block;
	int in_permutations;
	int yielded;

	size_t photon_size;
	void *left;
	void *right;

	long long min_pulse_distance;
	long long max_pulse_distance;
	long long min_time_distance;
	long long max_time_distance;

	queue_t *queue;

	index_offsets_t *index_offsets;
	permutation_t *permutation;
	correlation_t *correlation;

	correlate_t correlate;
	correlation_print_t correlation_print;

	int (*under_max_distance)(void const *correlator);
	int (*over_min_distance)(void const *correlator);
} correlator_t;

correlator_t *correlator_alloc(int const mode, unsigned int const order,
		size_t const queue_size, int const positive_only,
		long long const min_time_distance, long long const max_time_distance,
		long long const min_pulse_distance, 
		long long const max_pulse_distance);
int correlator_init(correlator_t *correlator);
void correlator_free(correlator_t **correlator);

int correlator_push(correlator_t *correlator, void const *photon);
int correlator_next(correlator_t *correlator);

int correlator_next_block(correlator_t *correlator);
int correlator_next_from_block(correlator_t *correlator);

int correlator_valid_distance(correlator_t *correlator);
int correlator_build_correlation(correlator_t *correlator);
void correlator_flush(correlator_t *correlator);

#endif
