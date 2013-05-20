#include <stdlib.h>

#include "temper.h"
#include "../modes.h"
#include "../error.h"
#include "../queue.h"

#include "t2.h"
#include "t3.h"

photon_stream_temper_t *photon_stream_temper_alloc(int const mode,
		unsigned int const channels, size_t const queue_length) {
	photon_stream_temper_t *pst = NULL;

	pst = (photon_stream_temper_t *)malloc(sizeof(photon_stream_temper_t));

	if ( pst == NULL ) {
		return(pst);
	}

	pst->mode = mode;
	pst->channels = channels;
	
	if ( pst->mode == MODE_T2 ) {
		debug("Mode t2\n");
		pst->photon_size = sizeof(t2_t);
		pst->photon_next = t2v_fscanf;
		pst->photon_print = t2v_fprintf;
		pst->photon_offset = t2v_offset;
		pst->channel_dim = t2v_channel_dimension;
		pst->window_dim = t2v_window_dimension;
	} else if ( pst->mode == MODE_T3 ) {
		debug("Mode t3.\n");
		pst->photon_size = sizeof(t3_t);
		pst->photon_next = t3v_fscanf;
		pst->photon_print = t3v_fprintf;
		pst->photon_offset = t3v_offset;
		pst->channel_dim = t3v_channel_dimension;
		pst->window_dim = t3v_window_dimension;
	} else {
		error("Invalid mode: %d\n", pst->mode);
		photon_stream_temper_free(&pst);
		return(pst);
	}

	pst->yielded_all_sorted = 1;

	debug("Allocating photons.\n");
	pst->current_photon = malloc(pst->photon_size);
	pst->left = malloc(pst->photon_size);
	pst->right = malloc(pst->photon_size);

	pst->suppress_channels = false;
	pst->suppressed_channels = (int *)malloc(pst->channels*sizeof(int));

	if ( pst->current_photon == NULL ||
			pst->left == NULL || pst->right == NULL || 
			pst->suppressed_channels == NULL ) {
		error("Could not allocate photons.\n");
		photon_stream_temper_free(&pst);
		return(pst);
	}

	debug("Allocating offsets.\n");
	pst->offsets = offsets_alloc(pst->channels);
	debug("Allocating queue.\n");
	pst->queue = queue_alloc(pst->photon_size, queue_length);

	if ( pst->offsets == NULL || pst->queue == NULL ) {
		error("Could not allocate offsets or queue.\n");
		photon_stream_temper_free(&pst);
		return(pst);
	}

	debug("Finished pst alloc.\n");
	return(pst);
}

void photon_stream_temper_init(photon_stream_temper_t *pst,
		FILE *stream_in, 
		int const filter_afterpulsing,
		int const suppress_channels, int const *suppressed_channels,
		int const offset_time, long long const *time_offsets,
		int const offset_pulse, long long const *pulse_offsets) {
	int i;

	pst->stream_in = stream_in;

	pst->filter_afterpulsing = filter_afterpulsing;

	pst->suppress_channels = suppress_channels;

	for ( i = 0; i < pst->channels; i++ ) {
		if ( suppress_channels ) {
			pst->suppressed_channels[i] = suppressed_channels[i];
		}
	}

	offsets_init(pst->offsets, 
			offset_time, time_offsets,
			offset_pulse, pulse_offsets);

	if ( pst->mode == MODE_T2 ) {
		pst->offset_span = offset_span(pst->offsets->time_offsets,
				pst->channels);
		queue_set_comparator(pst->queue, t2v_compare);
	} else {
		pst->offset_span = offset_span(pst->offsets->pulse_offsets,
				pst->channels);
		queue_set_comparator(pst->queue, t3v_compare);
	}
}

/* Yield a photon from the queue. If values can be taken from the queue,
 * yield them. Otherwise, fill it up with new values and sort it, then try
 * to yield a value.
 */
int photon_stream_temper_next(photon_stream_temper_t *pst) {
	int result;
	long long diff;

	while ( 1 ) {
		debug("Photons in the queue: %zu (of %zu)\n", 
				queue_size(pst->queue),
				queue_capacity(pst->queue));

		if ( feof(pst->stream_in) ) {
			debug("EOF for stream_in.\n");
			if ( queue_empty(pst->queue) ) {
				debug("Queue empty.\n");
				return(EOF);
			} else {
				debug("Popping a photon from queue.\n");
				queue_pop(pst->queue, pst->current_photon);
				return(PC_SUCCESS);
			}
		} else {
			if ( pst->yielded_all_sorted ) {
				debug("Acquiring more photons for the queue.\n");
				result = photon_stream_temper_populate(pst);

				if ( result != PC_SUCCESS && result != EOF ) {
					return(result);
				}

				debug("Sorting the queue.\n");
				queue_sort(pst->queue);
				pst->yielded_all_sorted = 0;
			} else {
				debug("Trying to pop a photon.\n");

				if ( queue_empty(pst->queue) ) {
					pst->yielded_all_sorted = 1;
				} else {
					queue_front(pst->queue, pst->left);
					queue_back(pst->queue, pst->right);

					diff = pst->window_dim(pst->right) -
							pst->window_dim(pst->left);

					result = (diff >= pst->offset_span);

					if ( pst->filter_afterpulsing &&
							pst->mode == MODE_T3 && 
							(((t3_t *)pst->left)->pulse == 
								((t3_t *)pst->right)->pulse) ) {
						debug("Filtering afterpulsing but still on a pulse.\n");
						pst->yielded_all_sorted = 1;
					} else if ( diff >= pst->offset_span ) {
						debug("Found a photon outside the offset bounds\n");
						queue_pop(pst->queue,
								pst->current_photon);
						return(PC_SUCCESS);
					} else {
						debug("Within the offset bounds, get more photons\n");
						pst->yielded_all_sorted = 1;
					}
				}
			}
		}
	}
}

int photon_stream_temper_populate(photon_stream_temper_t *pst) {
/* Fill the queue with photons from the stream. */
	int result;
	int suppress = false;
	int channel;
	int i;
	t3_t t3;

	while ( true ) {
		result = pst->photon_next(pst->stream_in, pst->current_photon);

		if ( result == EOF ) {
			debug("End of the stream.\n");
			break;
		} else if ( result != PC_SUCCESS ) {
			error("Error while reading photons: %d\n", result);
			return(result);
		}

		channel = pst->channel_dim(pst->current_photon);
		if ( channel < pst->channels ) {
			suppress = pst->suppress_channels && 
					pst->suppressed_channels[channel];
		} else {
			if ( pst->suppress_channels ) {
				error("Suppressing channels, but unspecified channel %d was "
						"found. Check channel specifications.\n", channel);
				return(PC_ERROR_CHANNEL);
			}
		}

		if ( ! suppress && pst->mode == MODE_T3 && pst->filter_afterpulsing ) {
			for ( i = 0; i < queue_size(pst->queue); i++ ) {
				queue_index(pst->queue, &t3, i);
				if ( t3.pulse == ((t3_t *)pst->current_photon)->pulse &&
						t3.channel == ((t3_t *)pst->current_photon)->channel ) {
					suppress = true;
					break;
				}
			}
		}

		if ( ! suppress ) {
			debug("Adding a photon on channel %d.\n", channel);
			pst->photon_offset(pst->current_photon, pst->offsets);
			
			result = queue_push(pst->queue, pst->current_photon);
			return(result);
		} else {
			debug("Suppressed a photon on channel %d\n", channel);
		}
	}

	return(result);
}

void photon_stream_temper_free(photon_stream_temper_t **pst) {
	if ( *pst != NULL ) {
		free((*pst)->current_photon);
		free((*pst)->left);
		free((*pst)->right);
		free((*pst)->suppressed_channels);
		offsets_free(&(*pst)->offsets);
		queue_free(&(*pst)->queue);
		free(*pst);
	}
}

int photon_temper(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
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
	int result = PC_SUCCESS;
	photon_stream_temper_t *pst;

	debug("Allocating offset photon stream.\n");
	pst = photon_stream_temper_alloc(options->mode, options->channels,
			options->queue_size);

	if ( pst == NULL ) {
		result = PC_ERROR_MEM;
	} 

	if ( result == PC_SUCCESS ) {
		photon_stream_temper_init(pst, stream_in,
				options->filter_afterpulsing,
				options->suppress_channels, options->suppressed_channels,
				options->offset_time, options->time_offsets,
				options->offset_pulse, options->pulse_offsets);

		while ( photon_stream_temper_next(pst) == PC_SUCCESS ) {
			pst->photon_print(stream_out, pst->current_photon);
		}
	}

	debug("Freeing pst\n");
	photon_stream_temper_free(&pst);
	return(PC_SUCCESS);
}
