#ifndef PHOTON_H_
#define PHOTON_H_

#include <stdio.h>
#include "types.h"
#include "options.h"

typedef struct {
	size_t length;
	size_t left_index;
	size_t right_index;
	size_t photon_size;
	void *queue;
	int empty;
	int mode;
	int (*compare)(const void *, const void *);
} photon_queue_t;

photon_queue_t *photon_queue_alloc(size_t const length, int const mode);
void photon_queue_init(photon_queue_t *queue);
void photon_queue_free(photon_queue_t **queue);
int photon_queue_full(photon_queue_t const *queue);
int photon_queue_empty(photon_queue_t const *queue);
int photon_queue_pop(photon_queue_t *queue, void *photon);
int photon_queue_push(photon_queue_t *queue, void *photon);
int photon_queue_front(photon_queue_t const *queue, void *photon);
int photon_queue_back(photon_queue_t const *queue, void *photon);
size_t photon_queue_index(photon_queue_t const *queue, void *photon, 
		int const index);
size_t photon_queue_size(photon_queue_t const *queue);
void photon_queue_sort(photon_queue_t *queue);

typedef struct {
	int64_t lower;
	int64_t upper;
} window_t;

typedef struct {
	float64_t lower;
	float64_t upper;
} fwindow_t;

typedef struct {
	window_t limits;
	int64_t width;
	int set_lower_bound;
	int set_upper_bound;
	int64_t lower_bound;
	int64_t upper_bound;
} photon_window_t;

void photon_window_init(photon_window_t *window, options_t const *options);
int photon_window_next(photon_window_t *window);
int photon_window_contains(photon_window_t const *window, int64_t const value);


typedef int (*photon_next_t)(FILE *, void *);
typedef int (*photon_print_t)(FILE *, void const *);

typedef int64_t (*photon_window_dimension_t)(void const *);
typedef int64_t (*photon_channel_dimension_t)(void const *);

typedef struct {
	FILE *stream_in;

	int mode;
	size_t photon_size;
	void *current_photon;
	int yielded_photon;

	photon_next_t photon_next;
	photon_print_t photon_print;

	photon_window_dimension_t window_dim;
	photon_channel_dimension_t channel_dim;
	photon_window_t window;

	int (*photon_stream_next)(void *photon_stream);
} photon_stream_t;

photon_stream_t *photon_stream_alloc(options_t const *options);
int photon_stream_init(photon_stream_t *photon_stream,
		FILE *stream_in, options_t const *options);
void photon_stream_free(photon_stream_t **photons);
int photon_stream_next_photon(photon_stream_t *photons);
int photon_stream_next_window(photon_stream_t *photons);

int photon_stream_next_windowed(void *photon_stream);
int photon_stream_next_unbounded(void *photon_stream);

int photon_stream_eof(photon_stream_t *photons);

int photon_echo(FILE *stream_in, FILE *stream_out, options_t const *options);

#endif
