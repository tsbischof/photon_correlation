#ifndef T2_H_
#define T2_H_

#include <stdio.h>
#include "options.h"

typedef struct {
	int channel;
	long long int time;
} t2_t;

typedef struct {
	int length;
	long long int left_index;
	long long int right_index;
	t2_t *queue;
} t2_queue_t;

int next_t2(FILE *in_stream, t2_t *record);
void print_t2(FILE *out_stream, t2_t *record);

t2_queue_t *allocate_t2_queue(int queue_length);
int next_t2_queue(FILE *in_stream,
		long long int max_time_distance,
		t2_queue_t *queue, options_t *options);
void free_t2_queue(t2_queue_t **queue);
t2_t get_queue_item_t2(t2_queue_t *queue, int index);

/* The windowed stream represents a series of streams which are composed of
 * photons in some time window. For example, an intensity stream can be 
 * generated by counting all photons in 50ms bins, so we want to be able
 * to produce all photons in consecutive 50ms windows.
 */
typedef struct {
	window_t limits;
	long long int width;
	int set_time_limit;
	long long int time_limit;
} t2_window_t;

typedef struct {
	t2_t current_photon;
	int yielded_photon;
	t2_window_t window;
	FILE *in_stream;
} t2_windowed_stream_t;

void init_t2_window(t2_window_t *window, 
		long long int start_time, options_t *options);
void next_t2_window(t2_window_t *window);
int init_t2_windowed_stream(t2_windowed_stream_t *stream,
		FILE *in_stream, options_t *options);
int next_t2_windowed(t2_windowed_stream_t *stream, t2_t *record,
		options_t *options);

#endif
