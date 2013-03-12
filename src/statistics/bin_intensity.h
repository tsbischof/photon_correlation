#ifndef BIN_INTENSITY_H_
#define BIN_INTENSITY_H_

#include <stdio.h>
#include "options.h"

typedef struct {
	int mode;
	unsigned int channels;
	
	limits_t window_limits;
	int window_scale;
	photon_window_t photon_window;

	edges_t *edges;
	uint64_t **counts;

	photon_channel_dimension_t channel_dim;
	photon_window_dimension_t window_dim;

	photon_queue_t *photon_queue;

	size_t photon_size;
	void *front;
	void *back;
	void *current;

	int64_t maximum_delay;
} bin_counts_photon_t;

bin_counts_photon_t *bin_counts_photon_alloc(options_t const *options);
void bin_counts_photon_init(bin_counts_photon_t *bin_counts,
		photon_window_t *photon_window);
void bin_counts_photon_free(bin_counts_photon_t **bin_counts);
int bin_counts_photon_increment(bin_counts_photon_t *bin_counts);
void bin_counts_photon_flush(bin_counts_photon_t *bin_counts);
int bin_counts_photon_push(bin_counts_photon_t *bin_counts, 
		void const *photon);

int bin_counts_photon_fprintf(FILE *stream_out, 
		bin_counts_photon_t const *bin_counts);

int bin_intensity_photon(FILE *stream_in, FILE *stream_out, 
		options_t const *options);

#endif

int bin_intensity(FILE *stream_in, FILE *stream_out, 
		options_t const *options);

#endif
