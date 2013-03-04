#include <stdlib.h>
#include <string.h>

#include "bin_intensity_photon.h"
#include "photon.h"
#include "error.h"
#include "intensity.h"
#include "modes.h"
#include "t2_void.h"
#include "t3_void.h"

bin_counts_photon_t *bin_counts_photon_alloc(options_t const *options) {
	int i;
	bin_counts_photon_t *bin_counts = NULL;

	bin_counts = (bin_counts_photon_t *)malloc(sizeof(bin_counts_photon_t));

	if ( bin_counts == NULL ) {
		return(bin_counts);
	}

	bin_counts->mode = options->mode;
	bin_counts->channels = options->channels;

	if ( options->mode == MODE_T2 ) {
		bin_counts->photon_size = sizeof(t2_t);
		bin_counts->channel_dim = t2v_channel_dimension;
		bin_counts->window_dim = t2v_window_dimension;
		memcpy(&(bin_counts->window_limits),
				&(options->time_limits),
				sizeof(limits_t));
		bin_counts->window_scale = options->time_scale;
	} else if ( options->mode == MODE_T3 ) {
		bin_counts->photon_size = sizeof(t3_t);
		bin_counts->channel_dim = t3v_channel_dimension;
		bin_counts->window_dim = t3v_window_dimension;
		memcpy(&(bin_counts->window_limits),
				&(options->pulse_limits),
				sizeof(limits_t));
		bin_counts->window_scale = options->pulse_scale;
	} else {
		error("Unrecognized mode: %d\n", options->mode);
		bin_counts_photon_free(&bin_counts);
		return(bin_counts);
	}

	bin_counts->front = malloc(bin_counts->photon_size);
	bin_counts->back = malloc(bin_counts->photon_size);
	bin_counts->current = malloc(bin_counts->photon_size);

	if ( bin_counts->front == NULL ||
			bin_counts->back == NULL || 
			bin_counts->current == NULL ) {
		error("Could not allocate photons for bin counts.\n");
		bin_counts_photon_free(&bin_counts);
		return(bin_counts);
	}

	bin_counts->photon_queue = photon_queue_alloc(options->queue_size, 
			bin_counts->mode);

	if ( bin_counts->photon_queue == NULL ) {
		error("Could not allocate photon queue for bin counts.\n");
		bin_counts_photon_free(&bin_counts);
		return(bin_counts);
	}

	bin_counts->edges = edges_alloc(bin_counts->window_limits.bins);
	bin_counts->counts = (uint64_t **)malloc(
			sizeof(uint64_t *)*bin_counts->window_limits.bins);

	if ( bin_counts->edges == NULL || bin_counts->counts == NULL ) {
		error("Could not allocate edges for bin counts.\n");
		bin_counts_photon_free(&bin_counts);
		return(bin_counts);
	}

	for ( i = 0; i < bin_counts->window_limits.bins; i++ ) {
		bin_counts->counts[i] = (uint64_t *)malloc(
				sizeof(uint64_t)*bin_counts->channels);

		if ( bin_counts->counts[i] == NULL ) {
			error("Could not allocate edges or counts for bin counts.\n");
			bin_counts_photon_free(&bin_counts);
			return(bin_counts);
		}
	}

	return(bin_counts);
}

void bin_counts_photon_init(bin_counts_photon_t *bin_counts, 
		photon_window_t *photon_window) {
	edges_init(bins_counts->edges, &(bins_counts->window_limits),
			bin_counts->window_scale);

	bin_counts->photon_window = photon_window;
}

void bin_counts_photon_free(bin_counts_photon_t **bin_counts) {
	int i;

	if ( *bin_counts != NULL ) {
		photon_queue_free(&((*bin_counts)->photon_queue));
		free((*bin_counts)->front);
		free((*bin_counts)->back);
		free((*bin_counts)->current);
		edges_free((*bin_counts)->edges);

		for ( i = 0; (*bin_counts)->counts != NULL &&
				i < (*bin_counts)->window_limits.bins; i++ ) {
			free((*bin_counts)->counts[i]);
		}
		free((*bin_counts)->counts);

		free(*bin_counts);
	}
}

int bin_counts_photon_increment(bin_counts_photon_t *bin_counts) {
/* Increment the first photon in the queue. */
	int i;
	float64_t lower, upper;

	photon_queue_pop(bin_counts->photon_queue, bin_counts->current);

	if ( bin_counts->photon_window.set_lower_bound ) {
		lower = (float64_t)bin_counts->photon_window.lower_bound;
	} else {
		photon_queue_index(bin_counts->photon_queue
		lower = 
		

	for ( i = 0; i < bin_counts->edges.bins; i++ ) {
}

void bin_counts_photon_flush(bin_counts_photon_t *bin_counts) {
/* Increment the entire queue. Use this when the stream reaches EOF. */
	while ( ! photon_queue_empty(bin_counts->photon_queue) ) {
		if ( bin_counts_photon_increment(bin_counts) != PC_SUCCESS ) {
			break;
		}
	}
}

int bin_counts_photon_push(bin_counts_photon_t *bin_counts, 
		void const *photon) {
/* Push a photon into the queue, then see if it is possible to increment. */
	int64_t front, back;

	if ( photon_queue_push(bin_counts->photon_queue, photon) != PC_SUCCESS ) {
		error("Could not add photon to queue.\n");
		return(PC_ERROR_QUEUE_OVERFLOW);
	}

	while ( 1 ) {
		photon_queue_front(bin_counts->photon_queue, bin_counts->front);
		photon_queue_back(bin_counts->photon_queue, bin_counts->back);
		front = bin_counts->window_dim(bin_counts->front);
		back = bin_counts->window_dim(bin_counts->back);

		if ( (front - back) > bin_counts->maximum_delay ) {
			bin_counts_photon_increment(bin_counts);
		} else {
			break;
		}
	}

	return(PC_SUCCESS);
}

int bin_counts_photon_fprintf(FILE *stream_out, 
			bin_counts_photon_t const *bin_counts) {
	return(PC_ERROR_IO);
}

int bin_intensity_photon(FILE *stream_in, FILE *stream_out, 
		options_t const *options) {
	int result;
	bin_counts_photon_t *bin_counts;
	photon_stream_t *photon_stream;

	bin_counts = bin_counts_photon_alloc(options);
	photon_stream = photon_stream_alloc(options);

	if ( bin_counts == NULL || photon_stream == NULL ) {
		error("Error while allocating bin counts and photon stream.\n");
		bin_counts_photon_free(&bin_counts);
		photon_stream_free(&photon_stream);
		return(PC_ERROR_MEM);
	}

	debug("Initializing.\n");
	bin_counts_photon_init(bin_counts);
	result = photon_stream_init(photon_stream, stream_in, options);

	if ( result == PC_SUCCESS ) {
		debug("Successfully initialized, processing stream.\n");
		while ( result == PC_SUCCESS && 
				photon_stream_next_photon(photon_stream) == PC_SUCCESS ) {
			result = bin_counts_photon_push(bin_counts, 
					photon_stream->current_photon);
		}

		bin_counts_photon_flush(bin_counts);
		bin_counts_photon_fprintf(stream_out, bin_counts);
	} else {
		error("Failed while initializing photon stream.\n");
	}

	debug("Freeing.\n");
	bin_counts_photon_free(&bin_counts);
	photon_stream_free(&photon_stream);

	return(PC_SUCCESS);
}
