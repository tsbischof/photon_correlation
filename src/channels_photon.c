#include <stdlib.h>

#include "error.h"
#include "channels.h"
#include "channels_photon.h"
#include "photon.h"
#include "modes.h"
#include "t2.h"
#include "t2_void.h"
#include "t3.h"
#include "t3_void.h"

photon_stream_offset_t *photon_stream_offset_alloc(options_t const *options) {
	photon_stream_offset_t *pso = NULL;

	pso = (photon_stream_offset_t *)malloc(sizeof(photon_stream_offset_t));

	if ( pso == NULL ) {
		return(pso);
	}

	pso->mode = options->mode;
	pso->channels = options->channels;
	
	if ( pso->mode == MODE_T2 ) {
		debug("Mode t2\n");
		pso->photon_size = sizeof(t2_t);
		pso->photon_next = t2v_fscanf;
		pso->photon_print = t2v_fprintf;
		pso->photon_offset = t2v_offset;
		pso->channel_dim = t2v_channel_dimension;
		pso->window_dim = t2v_window_dimension;
	} else if ( pso->mode == MODE_T3 ) {
		debug("Mode t3.\n");
		pso->photon_size = sizeof(t3_t);
		pso->photon_next = t3v_fscanf;
		pso->photon_print = t3v_fprintf;
		pso->photon_offset = t3v_offset;
		pso->channel_dim = t3v_channel_dimension;
		pso->window_dim = t3v_window_dimension;
	} else {
		error("Invalid mode: %d\n", pso->mode);
		photon_stream_offset_free(&pso);
		return(pso);
	}

	pso->yielded_all_sorted = 1;

	debug("Allocating photons.\n");
	pso->current_photon = malloc(pso->photon_size);
	pso->left = malloc(pso->photon_size);
	pso->right = malloc(pso->photon_size);

	pso->suppress_channels = options->suppress_channels;
	pso->suppressed_channels = (int *)malloc(pso->channels*sizeof(int));

	if ( pso->current_photon == NULL ||
			pso->left == NULL || pso->right == NULL || 
			pso->suppressed_channels == NULL ) {
		error("Could not allocate photons.\n");
		photon_stream_offset_free(&pso);
		return(pso);
	}

	debug("Allocating offsets.\n");
	pso->offsets = offsets_alloc(pso->channels);
	debug("Allocating queue.\n");
	pso->photon_queue = photon_queue_alloc(options->queue_size, pso->mode);

	if ( pso->offsets == NULL || pso->photon_queue == NULL ) {
		error("Could not allocate offsets or queue.\n");
		photon_stream_offset_free(&pso);
		return(pso);
	}

	debug("Finished pso alloc.\n");
	return(pso);
}

void photon_stream_offset_init(photon_stream_offset_t *pso,
		FILE *stream_in, options_t const *options) {
	int i;

	debug("Initializing pso.\n");

	pso->stream_in = stream_in;

	debug("Getting max offset.\n");
	if ( pso->mode == MODE_T2 ) {
		pso->max_offset_difference = offset_difference(
				pso->offsets->time_offsets, pso->channels);
	} else if ( pso->mode == MODE_T3 ) {
		pso->max_offset_difference = offset_difference(
				pso->offsets->pulse_offsets, pso->channels);
	}

	debug("Getting suppressed channels\n");
	for ( i = 0; pso->suppress_channels && i < pso->channels; i++ ) {
		pso->suppressed_channels[i] = options->suppressed_channels[i];
	}

	debug("Offsets init.\n");
	offsets_init(pso->offsets, options);

	debug("Finished pso init.\n");
}

/* Yield a photon from the queue. If values can be taken from the queue,
 * yield them. Otherwise, fill it up with new values and sort it, then try
 * to yield a value.
 */
int photon_stream_offset_next(photon_stream_offset_t *pso) {
	int result;
	int64_t diff;

	while ( 1 ) {
		debug("Photons in the queue: %zu (of %zu)\n", 
				photon_queue_size(pso->photon_queue),
				pso->photon_queue->length);
		if ( feof(pso->stream_in) ) {
			debug("EOF for stream_in.\n");
			if ( photon_queue_empty(pso->photon_queue) ) {
				debug("Queue empty.\n");
				return(EOF);
			} else {
				debug("Popping a photon from queue.\n");
				photon_queue_pop(pso->photon_queue, pso->current_photon);
				return(PC_SUCCESS);
			}
		} else {
			if ( pso->yielded_all_sorted ) {
				debug("Acquiring more photons for the queue.\n");
				result = photon_stream_offset_populate(pso);

				if ( result != PC_SUCCESS && result != EOF ) {
					return(result);
				}

				debug("Sorting the queue.\n");
				photon_queue_sort(pso->photon_queue);
				pso->yielded_all_sorted = 0;
			} else {
				debug("Trying to pop a photon.\n");

				if ( photon_queue_empty(pso->photon_queue) ) {
					pso->yielded_all_sorted = 1;
				} else {
					photon_queue_front(pso->photon_queue, pso->left);
					photon_queue_back(pso->photon_queue, pso->right);

					diff = pso->window_dim(pso->right) -
							pso->window_dim(pso->left);
	
					if ( diff >= pso->max_offset_difference ) {
						debug("Found a photon outside the offset bounds\n");
						photon_queue_pop(pso->photon_queue,
								pso->current_photon);
						return(PC_SUCCESS);
					} else {
						debug("Within the offset bounds, get more photons\n");
						if ( photon_queue_full(pso->photon_queue) ) {
							error("Photon queue is full, extend its size "
									"to continue with the calculation. "
									"Current size: %zu\n", 
									pso->photon_queue->length);
							return(PC_ERROR_QUEUE_OVERFLOW);
						}

						pso->yielded_all_sorted = 1;
					}
				}
			}
		}
	}
}

int photon_stream_offset_populate(photon_stream_offset_t *pso) {
/* Fill the queue with photons from the stream. */
	int result;
	int suppress;
	int channel;

	while ( ! photon_queue_full(pso->photon_queue) ) {
		result = pso->photon_next(pso->stream_in, pso->current_photon);

		if ( result == EOF ) {
			debug("End of the stream.\n");
			break;
		} else if ( result != PC_SUCCESS ) {
			error("Error while reading photons: %d\n", result);
			return(result);
		}

		channel = pso->channel_dim(pso->current_photon);
		if ( channel < pso->channels ) {
			suppress = pso->suppress_channels && 
					pso->suppressed_channels[channel];
		} else {
			if ( pso->suppress_channels ) {
				error("Suppressing channels, but unspecified channel %d was "
						"found. Check channel specifications.\n", channel);
				return(PC_ERROR_CHANNEL);
			}
		}

		if ( ! suppress ) {
			debug("Adding a photon on channel %d.\n", channel);
			pso->photon_offset(pso->current_photon, pso->offsets);
			photon_queue_push(pso->photon_queue, pso->current_photon);
		} else {
			debug("Suppressed a photon on channel %d\n", channel);
		}
	}

	return(result);
}

void photon_stream_offset_free(photon_stream_offset_t **pso) {
	if ( *pso != NULL ) {
		free((*pso)->current_photon);
		free((*pso)->left);
		free((*pso)->right);
		free((*pso)->suppressed_channels);
		offsets_free(&(*pso)->offsets);
		photon_queue_free(&(*pso)->photon_queue);
		free(*pso);
	}
}

int channels_photon(FILE *stream_in, FILE *stream_out, 
		options_t const *options) {
	/* This needs to do two things:
	 * 1. Suppress photons arriving on particular channels.
	 * 2. Retain those photons on the other channels, and apply any time 
	 *    offsets they require. 
	 * 
	 * The first step is easy: we just choose not to add the photon to the
	 * queue of new photons. The second one requires a little more work, 
	 * but can be divided into two steps:
	 * 1. Allocate a fixed array to populate with photons.
	 * 2. For each new photon, add it to the end of the array.
	 * 3. When the array is full, sort it and emit all photons possible.
	 * 4. If the array is full and photons cannot be removed, report that the 
	 *    array (queue) size is not large enough.
	 * 5. At the end of the stream, sort and emit all photons.
	 */
	photon_stream_offset_t *pso;

	debug("Allocating offset photon stream.\n");
	pso = photon_stream_offset_alloc(options);

	if ( pso == NULL ) {
		return(PC_ERROR_MEM);
	} 

	debug("Initializing stream.\n");
	photon_stream_offset_init(pso, stream_in, options);

	debug("Processing stream.\n");
	while ( photon_stream_offset_next(pso) == PC_SUCCESS ) {
		pso->photon_print(stream_out, pso->current_photon);
	}

	debug("Freeing pso\n");
	photon_stream_offset_free(&pso);
	return(PC_SUCCESS);
}
