#ifndef STREAM_H_
#define STREAM_H_

#include <stdio.h>
#include "photon.h"
#include "window.h"

typedef struct {
	FILE *stream_in;

	int mode;
	size_t photon_size;
	void *photon;
	int yielded;

	photon_next_t photon_next;
	photon_print_t photon_print;

	photon_window_dimension_t window_dim;
	photon_channel_dimension_t channel_dim;
	photon_window_t window;

	int (*photon_stream_next)(void *photon_stream);
} photon_stream_t;

photon_stream_t *photon_stream_alloc(int const mode);
void photon_stream_init(photon_stream_t *photon_stream, FILE *stream_in);
void photon_stream_free(photon_stream_t **photons);

void photon_stream_set_unwindowed(photon_stream_t *photons);
void photon_stream_set_windowed(photon_stream_t *photons,
		long long const bin_width,
		int const set_lower_bound, long long const lower_bound,
		int const set_upper_bound, long long const upper_bound);

int photon_stream_next_photon(photon_stream_t *photons);
int photon_stream_next_window(photon_stream_t *photons);

int photon_stream_next_windowed(void *photon_stream);
int photon_stream_next_unwindowed(void *photon_stream);

int photon_stream_eof(photon_stream_t *photons);

#endif
