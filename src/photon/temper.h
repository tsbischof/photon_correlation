#ifndef TEMPER_H_
#define TEMPER_H_

#include <stdio.h>

#include "photon.h"
#include "offsets.h"

#include "../options.h"
#include "../queue.h"

typedef struct {
	FILE *stream_in;

	int mode;
	unsigned int channels;
	size_t photon_size;
	void *current_photon;
	void *left;
	void *right;

	int suppress_channels;
	int *suppressed_channels;

	long long offset_span;
	offsets_t *offsets;

	queue_t *queue;
	int yielded_all_sorted;

	photon_next_t photon_next;
	photon_print_t photon_print;

	photon_offset_t photon_offset;
	photon_channel_dimension_t channel_dim;
	photon_window_dimension_t window_dim;
} photon_stream_temper_t;

photon_stream_temper_t *photon_stream_temper_t_alloc(int const mode,
		unsigned int const channels, size_t const queue_length);
void photon_stream_temper_init(photon_stream_temper_t *pst,
		FILE *stream_in, 
		int const suppress_channels, int const *suppressed_channels,
		int const offset_time, long long const *time_offsets,
		int const offset_pulse, long long const *pulse_offsets);
int photon_stream_temper_next(photon_stream_temper_t *pst);
void photon_stream_temper_free(photon_stream_temper_t **pst);
int photon_stream_temper_populate(photon_stream_temper_t *pst);

int photon_temper(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
