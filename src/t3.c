#include <stdio.h>
#include <stdlib.h>

#include "t3.h"

int next_t3(FILE *in_stream, t3_t *record) {
	int result;

	result = fscanf(in_stream, "%d,%lld,%u",
			&(record->channel),
			&(record->pulse_number),
			&(record->time));
	
	return(result != 3);
}

void print_t3(FILE *out_stream, t3_t *record) {
	fprintf(out_stream, "%d,%lld,%u", 
			record->channel,
			record->pulse_number,
			record->time);
}


t3_queue_t *allocate_t3_queue(int queue_length) {
	int result = 0;
	t3_queue_t *queue;

	queue = (t3_queue_t *)malloc(sizeof(t3_queue_t));
	if ( queue == NULL ) {
		result = -1;
	} else {
		queue->length = queue_length;
		queue->left_index = -1;
		queue->right_index = -1;
	
		queue->queue = (t3_t *)malloc(sizeof(t3_t)*queue->length);
		if ( queue->queue == NULL ) {
			result = -1;
		}
	}

	if ( result ) {
		free_t3_queue(&queue);
	}

	return(queue);
}

void free_t3_queue(t3_queue_t **queue) {
	if ( *queue != NULL ) {
		if ( (*queue)->queue != NULL ) {
			free((*queue)->queue);
		}
		free(*queue);
	}
}

t3_t get_queue_item_t3(t3_queue_t *queue, int index) {
	int true_index = (queue->left_index + index) % queue->length;
	return(queue->queue[true_index]);
}


/* These functions break up the photons into subsets by dividing time into
 * windows of fixed length. Thus we need to be able to receive a photon from a 
 * window, and to iterate between the windows.
 */
void init_t3_window(t3_window_t *window, options_t *options) {
	window->limits.lower = 0;
	window->limits.bins = 1;
	window->limits.upper = options->bin_width;
	window->width = options->bin_width;
}

void next_t3_window(t3_window_t *window) {
	window->limits.lower += window->width;
	window->limits.upper += window->width;
}

void init_t3_windowed_stream(t3_windowed_stream_t *stream, 
		FILE *in_stream, options_t *options) {
	stream->yielded_photon = 0;
	stream->in_stream = in_stream;
	stream->current_photon.channel = -1;
	init_t3_window(&(stream->window), options);
}

int next_t3_windowed(t3_windowed_stream_t *stream, t3_t *record,
		options_t *options) {
	if ( stream->yielded_photon ) {
		stream->current_photon.channel = -1;
	}
		
	if ( stream->current_photon.channel == -1 ) {
		if ( next_t3(stream->in_stream, &(stream->current_photon)) ) {
			return(-1);
		}
	}

	if ( (! options->count_all) 
			&& stream->current_photon.pulse_number
					 > stream->window.limits.upper ) {
		stream->yielded_photon = 0;
		return(1);
	} else if ( stream->current_photon.pulse_number
					 < stream->window.limits.lower ) {
		/* this helps prevent inifinite loops: imagine that we define the window
 		 * to start after the photon, at which point we keep moving forward 
 		 * without ever finding the correct window.
 		 */
		return(-2);
	} else {
		stream->yielded_photon = 1;
		
		record->channel = stream->current_photon.channel;
		record->pulse_number = stream->current_photon.pulse_number;
		record->time = stream->current_photon.time;
		return(0);
	}
}
