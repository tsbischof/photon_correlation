#ifndef PHOTON_H_
#define PHOTON_H_

#include <stdio.h>
#include "types.h"

typedef struct {
	size_t length;
	size_t left_index;
	size_t right_index;
	size_t photon_size;
	void *queue;
	int empty;
	int (*compare)(const void *, const void *);
} photon_queue_t;

photon_queue_t *photon_queue_alloc(size_t length, size_t photon_size);
void photon_queue_free(photon_queue_t **queue);
int photon_queue_full(photon_queue_t *queue);
int photon_queue_pop(photon_queue_t *queue, void *photon);
int photon_queue_push(photon_queue_t *queue, void *photon);
int photon_queue_front(photon_queue_t *queue, void *photon);
int photon_queue_back(photon_queue_t *queue, void *photon);
size_t photon_queue_index(photon_queue_t *queue, void *photon, int index);
size_t photon_queue_size(photon_queue_t *queue);
void photon_queue_sort(photon_queue_t *queue);

typedef struct {
	uint64_t lower;
	uint64_t upper;
} window_t;

typedef struct {
	float64_t lower;
	float64_t upper;
} fwindow_t;

typedef struct {
	window_t limits;
	uint64_t width;
	int set_lower_bound;
	int set_upper_bound;
	uint64_t lower_bound;
	uint64_t upper_bound;
} photon_window_t;

typedef int (*photon_next_t)(FILE *, void *);
typedef int64_t (*photon_window_dimension_t)(const void *);
typedef int64_t (*photon_channel_dimension_t)(const void *);

typedef struct {
	size_t photon_size;
	void *current_photon;
	int yielded_photon;
	FILE *stream_in;
	photon_next_t photon_next;
	photon_window_dimension_t dim;
	photon_window_t window;
} photon_stream_t;

void photon_window_init(photon_window_t *window, 
		int set_lower_bound, int64_t lower_bound, 
		uint64_t width,
		int set_upper_bound, int64_t upper_bound);
int photon_window_next(photon_window_t *window);

typedef int (*photon_stream_next_t)(photon_stream_t *, void *);
int photon_stream_next_windowed(photon_stream_t *photons, void *photon);
int photon_stream_next_unbounded(photon_stream_t *photons, void *photon);

int photon_stream_init(photon_stream_t *photons, 
		photon_stream_next_t *stream_next,
		photon_window_dimension_t dim,
		photon_next_t photon_next,
		size_t photon_size, FILE *stream_in,
		int set_lower_bound, int64_t lower_bound, 
		uint64_t width,
		int set_upper_bound, int64_t upper_bound);
void photon_stream_free(photon_stream_t *photons);
int photon_stream_next_window(photon_stream_t *photons);

#endif
