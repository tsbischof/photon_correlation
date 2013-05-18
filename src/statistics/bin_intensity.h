#ifndef BIN_INTENSITY_H_
#define BIN_INTENSITY_H_

#include <stdio.h>
#include "../options.h"
#include "../queue.h"
#include "../photon/photon.h"
#include "../photon/window.h"
#include "../limits.h"
#include "../histogram/edges.h"

typedef struct {
	int mode;
	unsigned int order;
	unsigned int channels;

	int flushing;
	
	limits_t window_limits;
	int window_scale;

	int set_start;
	long long start;
	int set_stop;
	long long stop;

	edges_t *edges;
	unsigned long long **counts;

	photon_channel_dimension_t channel_dim;
	photon_window_dimension_t window_dim;

	queue_t *queue;

	size_t photon_size;
	void *current;

	long long maximum_delay;
} bin_intensity_t;

bin_intensity_t *bin_intensity_alloc(int const mode, unsigned int const order,
		unsigned int const channels, 
		limits_t const *time_limits, int const time_scale,
		limits_t const *pulse_limits, int const pulse_scale,
		size_t const queue_size);
void bin_intensity_init(bin_intensity_t *bin_intensity,
		int set_start, long long start,
		int set_stop, long long stop);
void bin_intensity_free(bin_intensity_t **bin_intensity);
void bin_intensity_increment(bin_intensity_t *bin_intensity);
void bin_intensity_flush(bin_intensity_t *bin_intensity);
int bin_intensity_valid_distance(bin_intensity_t *bin_intensity);
int bin_intensity_push(bin_intensity_t *bin_intensity, void const *photon);

int bin_intensity_fprintf(FILE *stream_out, 
		bin_intensity_t const *bin_intensity);
int bin_intensity_fprintf_bins(FILE *stream_out, 
		bin_intensity_t const *bin_intensity, unsigned int const blanks);
int bin_intensity_fprintf_counts(FILE *stream_out, 
		bin_intensity_t const *bin_intensity);

int bin_intensity(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
