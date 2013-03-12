#ifndef INTENSITY_H_
#define INTENSITY_H_

#include <stdio.h>

#include "counts.h"
#include "../photon/window.h"
#include "../options.h"

typedef struct {
	unsigned int channels;

	counts_t *counts;

	photon_window_t window;

	int count_all;

	int first_photon_seen;
	int flushing;
	int record_available;
	int yielded;

	int photon_held;
	size_t photon_size;
	void *photon;

	photon_channel_dimension_t channel_dim;
	photon_window_dimension_t window_dim;

	long long last_window_seen;
} intensity_photon_t;

intensity_photon_t *intensity_photon_alloc(unsigned int const channels, 
		int const mode);
void intensity_photon_init(intensity_photon_t *intensity,
		int count_all,
		long long bin_width,
		int set_lower_bound, long long lower_bound,
		int set_upper_bound, long long upper_bound);
void intensity_photon_counts_init(intensity_photon_t *intensity);
void intensity_photon_free(intensity_photon_t **intensity);

int intensity_photon_push(intensity_photon_t *intensity, void const *photon);
int intensity_photon_increment(intensity_photon_t *intensity,
		unsigned int const channel);
int intensity_photon_next(intensity_photon_t *intensity);
void intensity_photon_flush(intensity_photon_t *intensity);

typedef int (*intensity_photon_next_t)(FILE *, intensity_photon_t *);
typedef int (*intensity_photon_print_t)(FILE *, intensity_photon_t const *);

int intensity_photon_fscanf(FILE *stream_in, intensity_photon_t *intensity);
int intensity_photon_fprintf(FILE *stream_out, 
		intensity_photon_t const *intensity);

int intensity_photon(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
