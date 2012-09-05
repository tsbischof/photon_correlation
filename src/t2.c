#include <stdlib.h>
#include <math.h>

#include "error.h"
#include "t2.h"

int next_t2(FILE *in_stream, t2_t *record, options_t *options) {
	int result;

	if ( options->binary_in ) {
		result = fread(record, sizeof(t2_t), 1, in_stream);
		result = (result != 1);
	} else {
		result = fscanf(in_stream, 
				"%"SCNd32",%"SCNd64,
			 	&(record->channel),
				&(record->time));
		result = (result != 2);
	}
	
	return(result);
}

void print_t2(FILE *out_stream, t2_t *record, int print_newline,
		options_t *options) {
	if ( options->binary_out ) {
		fwrite(record, sizeof(t2_t), 1, out_stream);
	} else {
		fprintf(out_stream, "%"PRId32",%"PRId64, 
				record->channel,
				record->time);

		if ( print_newline == NEWLINE ) {
			fprintf(out_stream, "\n");
		}
	}
}

int t2_comparator(const void *a, const void *b) {
	/* Comparator to be used with standard sorting algorithms to sort
	 * t2 photons. Yields 1 for sorted, 0 for equal, and -1 for reversed.
     */
	return(((t2_t *)a)->time < ((t2_t *)b)->time);
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
		int64_t max_time_distance,
		t2_queue_t *queue, options_t *options) {
	int64_t starting_index;
	int64_t ending_index;

	queue->left_index += 1;
	starting_index = queue->left_index % queue->length;
	ending_index = queue->right_index % queue->length;

	while ( 1 ) {
		if ( feof(in_stream) ) {
			/* We have reached the end of the stream, so keep moving the left
			 * index forward until the queue is empty.
			 */
			return(queue->left_index < queue->right_index);
		} else if ( ending_index > 0 && 
				( queue->queue[ending_index].time 
					- queue->queue[starting_index].time ) 
				< max_time_distance ) {
			/* Still within the distance bounds */
		} else {
			queue->right_index += 1;
			ending_index = queue->right_index % queue->length;
			
			if ( ! next_t2(in_stream, &(queue->queue[ending_index]), options) 
					&& ! feof(in_stream) ) {
				/* Failed to read a photon. We have already checked that we
 				 * we are not at the end of the stream, so we have a read
 				 * error on our hands.
 				 */
				error("Error while reading t2 stream.\n");
				return(0);
			} else if ( (queue->right_index - queue->left_index)
					>= queue->length ) {
				warn("Overflow of queue entries, increase queue size for "
					"accurate results.\n");
			}

			/* By here, we have no error and no EOF, so move along. */
		}
	}
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
 * windows of fixed length. Thus we need to be able to receive a photon from 
 * a window, and to iterate between the windows.
 */
void init_t2_window(t2_window_t *window, 
		int64_t start_time, options_t *options) {
	if ( options->set_start_time ) {
		window->limits.lower = options->start_time;
	} else {
		window->limits.lower = start_time;
	}

	window->limits.upper = options->bin_width*
			((int)floor(window->limits.lower/options->bin_width)+1);
	window->width = options->bin_width;

	window->set_time_limit = options->set_stop_time;
	window->time_limit = options->stop_time;

	if ( window->set_time_limit && window->limits.upper > window->time_limit ) {
		window->limits.upper = window->time_limit;
	}
}

void next_t2_window(t2_window_t *window) {
	window->limits.lower = window->limits.upper;
	window->limits.upper += window->width;

	if ( window->set_time_limit && window->limits.upper > window->time_limit ) {
		window->limits.upper = window->time_limit;
	}
}

int init_t2_windowed_stream(t2_windowed_stream_t *stream, 
		FILE *in_stream, options_t *options) {
	stream->yielded_photon = 0;
	stream->in_stream = in_stream;

	if ( next_t2(in_stream, &(stream->current_photon), options) ) {
		return(-1);
	} else {
		init_t2_window(&(stream->window), 
				stream->current_photon.time, options);
	}

	return(0);
}

int next_t2_windowed(t2_windowed_stream_t *stream, t2_t *record,
		options_t *options) {
	/* Deal with the actual photon now. */
	if ( stream->yielded_photon ) {
		stream->current_photon.channel = -1;
	}
		
	if ( stream->current_photon.channel == -1 ) {
		if ( next_t2(stream->in_stream, 
				&(stream->current_photon), options) ) {
			return(-1);
		}
	}

	/* Test that the photon is within the absolute limits */ 
	if ( options->set_start_time &&
			stream->current_photon.time < options->start_time ) {
		stream->yielded_photon = 1;
		return(2);
	} 

	if ( options->set_stop_time && 
			stream->current_photon.time >= options->stop_time ) {
		return(-1);
	}

	/* Process the photon normally. */
	if ( (! options->count_all) 
			&& stream->current_photon.time > stream->window.limits.upper ) {
		stream->yielded_photon = 0;
		return(1);
	} else if ( stream->current_photon.time < stream->window.limits.lower ) {
		/* this helps prevent inifinite loops: imagine that we define the 
		 * window to start after the photon, at which point we keep moving 
		 * forward without ever finding the correct window.
 		 */
		return(-2);
	} else {
		stream->yielded_photon = 1;
		
		record->channel = stream->current_photon.channel;
		record->time = stream->current_photon.time;
		return(0);
	}
}
