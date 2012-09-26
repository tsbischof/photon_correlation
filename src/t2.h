#ifndef T2_H_
#define T2_H_

#include <stdio.h>

#include "types.h"
#include "options.h"

typedef struct {
	int32_t channel;
	int64_t time;
} t2_t;

typedef struct {
	int32_t length;
	int64_t left_index;
	int64_t right_index;
	t2_t *queue;
} t2_queue_t;

int next_t2(FILE *in_stream, t2_t *record, options_t *options);
void print_t2(FILE *out_stream, t2_t *record, int print_newline,
		options_t *options);
int t2_comparator(const void *a, const void *b);

t2_queue_t *allocate_t2_queue(int queue_length);
void free_t2_queue(t2_queue_t **queue);
int t2_queue_full(t2_queue_t *queue);
int t2_queue_pop(t2_t *record, t2_queue_t *queue);
int t2_queue_push(t2_t *record, t2_queue_t *queue);
int t2_queue_front(t2_t *record, t2_queue_t *queue);
int t2_queue_back(t2_t *record, t2_queue_t *queue);
int t2_queue_index(t2_t *record, t2_queue_t *queue, int index);
int64_t t2_queue_size(t2_queue_t *queue);
void t2_queue_sort(t2_queue_t *queue);
void yield_t2_queue(FILE *out_stream, t2_queue_t *queue, options_t *options);

/* The windowed stream represents a series of streams which are composed of
 * photons in some time window. For example, an intensity stream can be 
 * generated by counting all photons in 50ms bins, so we want to be able
 * to produce all photons in consecutive 50ms windows.
 */
typedef struct {
	window_t limits;
	int64_t width;
	int set_time_limit;
	int64_t time_limit;
} t2_window_t;

typedef struct {
	t2_t current_photon;
	int yielded_photon;
	t2_window_t window;
	FILE *in_stream;
} t2_windowed_stream_t;

void init_t2_window(t2_window_t *window, 
		int64_t start_time, options_t *options);
void next_t2_window(t2_window_t *window);
int init_t2_windowed_stream(t2_windowed_stream_t *stream,
		FILE *in_stream, options_t *options);
int next_t2_windowed(t2_windowed_stream_t *stream, t2_t *record,
		options_t *options);

#endif
