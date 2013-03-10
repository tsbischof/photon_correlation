#ifndef CHANNELS_PHOTON_H_
#define CHANNELS_PHOTON_H_

#include <stdio.h>

#include "options.h"
#include "photon.h"

typedef struct {
	FILE *stream_in;

	int mode;
	int channels;
	size_t photon_size;
	void *current_photon;
	void *left;
	void *right;

	int suppress_channels;
	int *suppressed_channels;

	int64_t max_offset_difference;
	offsets_t *offsets;

	photon_queue_t *photon_queue;
	int yielded_all_sorted;

	photon_next_t photon_next;
	photon_print_t photon_print;

	photon_offset_t photon_offset;
	photon_channel_dimension_t channel_dim;
	photon_window_dimension_t window_dim;
} photon_stream_offset_t;

photon_stream_offset_t *photon_stream_offset_alloc(pc_options_t const *options);
void photon_stream_offset_init(photon_stream_offset_t *pso,
		FILE *stream_in, pc_options_t const *options);
int photon_stream_offset_next(photon_stream_offset_t *pso);
void photon_stream_offset_free(photon_stream_offset_t **pso);
int photon_stream_offset_populate(photon_stream_offset_t *pso);

int channels_photon(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
