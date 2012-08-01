#include <stdlib.h>

#include "t2.h"

int next_t2(FILE *in_stream, t2_t *record) {
	int result;

	result = fscanf(in_stream, "%d,%lld",
			&(record->channel),
			&(record->time));
	
	return(result != 2);
}

void print_t2(FILE *out_stream, t2_t *record) {
	fprintf(out_stream, "%d,%lld", 
			record->channel,
			record->time);
}

t2_queue_t *allocate_t2_queue(int queue_length) {
	int result = 0;
	t2_queue_t *queue;

	queue = (t2_queue_t *)malloc(sizeof(t2_queue_t));
	if ( queue == NULL ) {
		result = -1;
	} else {
		queue->length = queue_length;
		queue->left_index = -1;
		queue->right_index = -1;
	
		queue->queue = (t2_t *)malloc(sizeof(t2_t)*queue->length);
		if ( queue->queue == NULL ) {
			result = -1;
		}
	}

	if ( result ) {
		free_t2_queue(&queue);
	}

	return(queue);
}

int next_t2_queue(FILE *in_stream, 
		long long int max_time_distance,
		t2_queue_t *queue, options_t *options) {
	return(-1);
}

void free_t2_queue(t2_queue_t **queue) {
	if ( *queue != NULL ) {
		if ( (*queue)->queue != NULL ) {
			free((*queue)->queue);
		}
		free(*queue);
	}
}

t2_t get_queue_item_t2(t2_queue_t *queue, int index) {
	int true_index = (queue->left_index + index) % queue->length;
	return(queue->queue[true_index]);
}

/* These functions break up the photons into subsets by dividing time into
 * windows of fixed length. Thus we need to be able to receive a photon from a 
 * window, and to iterate between the windows.
 */
void init_t2_window(t2_window_t *window, options_t *options) {
	window->limits.lower = 0;
	window->limits.bins = 1;
	window->limits.upper = options->bin_width;
	window->width = options->bin_width;
}

void next_t2_window(t2_window_t *window) {
	window->limits.lower += window->width;
	window->limits.upper += window->width;
}

void init_t2_windowed_stream(t2_windowed_stream_t *stream, 
		FILE *in_stream, options_t *options) {
	stream->yielded_photon = 0;
	stream->in_stream = in_stream;
	stream->current_photon.channel = -1;
	init_t2_window(&(stream->window), options);
}

int next_t2_windowed(t2_windowed_stream_t *stream, t2_t *record,
		options_t *options) {
	if ( stream->yielded_photon ) {
		stream->current_photon.channel = -1;
	}
		
	if ( stream->current_photon.channel == -1 ) {
		if ( next_t2(stream->in_stream, &(stream->current_photon)) ) {
			return(-1);
		}
	}

	if ( (! options->count_all) 
			&& stream->current_photon.time > stream->window.limits.upper ) {
		stream->yielded_photon = 0;
		return(1);
	} else if ( stream->current_photon.time < stream->window.limits.lower ) {
		/* this helps prevent inifinite loops: imagine that we define the window
 		 * to start after the photon, at which point we keep moving forward 
 		 * without ever finding the correct window.
 		 */
		return(-2);
	} else {
		stream->yielded_photon = 1;
		
		record->channel = stream->current_photon.channel;
		record->time = stream->current_photon.time;
		return(0);
	}
}
