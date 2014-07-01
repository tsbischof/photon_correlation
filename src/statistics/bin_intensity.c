/*
 * Copyright (c) 2011-2014, Thomas Bischof
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Massachusetts Institute of Technology nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <math.h>

#include "bin_intensity.h"
#include "../modes.h"
#include "../error.h"
#include "../types.h"
#include "../photon/stream.h"
#include "../photon/t2.h"
#include "../photon/t3.h"

/* T2 and T3 in principle distinct, but I have not worked out the complete math
 * for T3 mode. For their primary (time/pulse, respectively) axes, the process
 * is identical, and is described here. The full result for T3 should handle
 * its time axis, but it is not clear how to do this, or even if it is
 * necessary.
 *
 * This program calculates the intensity of a time-offset stream of photons.
 * The idea here is that we have a function I(t) which is defined for some
 * limits [lower, upper), and we have some time offset dt. We wish to calculate
 * I(t+dt), but the limits of integration have not changed. As such, any 
 * intensity which falls outside the limits of time is not counted. 
 *
 * The time offsets given here are the same as those for a histogram, such that
 * there is a start, number of bins, an end, and a scale.
 *
 * We must follow the intensity stream (photons), keeping track of whether or
 * not the photon falls into some histogram bin, based on that bin's offset
 * from the limits of integration. In the most general case, these limits
 * are defined by the first and last photon seen, but these can also be 
 * specified externally. If a photon is sufficiently far from the limits of
 * integration to be counted, it is, Otherwise, it is ignored.
 *
 * Thus there are three distinct classes of photons:
 * 1. Photon at the beginning of the stream (less far from the beginning than
 *    the longest delay permissible).
 * 2. Photon at the end of the stream (less far from the end than the longest
 *    delay permissible).
 * 3. Photon in the middle (all other photons).
 *
 * Graphically, this looks something like:
 * ends:        |<---------->|                              |<---------->|
 * photons:     *  *       *    * *          *        *                  *
 * class:       |<-  start ->|<---------- middle ---------->|<-- end  -->|
 *
 * As such, we must keep track of the current limits of integration time, 
 * and use those to determine whether the given time bin permits inclusion
 * of the given photon. The general algorithm will look something like:
 *
 * photon_queue = photons.pop()
 * max_delay = max(limits)
 * while photons:
 *     if photon_queue.back().time - photon_queue.front().time > max_delay:
 *         increment_bins(bins, photon_queue.pop(),
 *                        photon_queue.first_photon
 *                        photon_queue.back())
 *     else:
 *         photon_queue.push(photons.pop())
 *
 * while photon_queue:
 *     increment_bins(bins, photon_queue.pop(),
 *                    photon_queue.first_photon,
 *                    photon_queue.back())
 *
 * print_bins(bins)
 *
 *
 *
 * The logic behind the back-front>max_delay line is as follows: if a photon
 * is at least as far from the ends as the maximal displacement, it must be
 * able to be counted in any bin, and as such is ready for processing. This 
 * helps keep us from counting a photon as not included, when it really could
 * be when the next photon arrives.
 *
 * The increment process involves checking each bin to see whether the photon 
 * can be included, based on the first and last photons seen (or the start
 * and end of the integration time, as may be the case). Here the back() call
 * refers to the final photon, but in practice logic is used to check whether
 * the true time is the photon time or a specified time. The increment 
 * algorithm looks like:
 *
 * def increment_bins(bins, current_photon, front, back):
 *     for bin in bins:
 *         if (current_photon.time + bin.lower) in range(front, back)
 *             or (current_photon.time + bin.upper) in range(front, back):
 *             bin.increment()
 *
 *
 *
 * All of this is for T2 photons. The T3 pulse dimension is identical, but I
 * have not yet worked out how to deal with the time dimension. As such,
 * only the pulse dimension is worked out so far.
 */


bin_intensity_t *bin_intensity_alloc(int const mode, unsigned int const order,
		unsigned int const channels,
		limits_t const *time_limits, int const time_scale,
		limits_t const *pulse_limits, int const pulse_scale,
		size_t const queue_size) {
	int i;
	bin_intensity_t *bin_intensity = NULL;

	bin_intensity = (bin_intensity_t *)malloc(sizeof(bin_intensity_t));

	if ( bin_intensity == NULL ) {
		return(bin_intensity);
	}

	bin_intensity->mode = mode;
	bin_intensity->order = order;
	bin_intensity->channels = channels;

	if ( mode == MODE_T2 ) {
		bin_intensity->window_scale = time_scale;
		memcpy(&(bin_intensity->window_limits),
				time_limits,
				sizeof(limits_t));
		bin_intensity->channel_dim = t2_channel_dimension;
		bin_intensity->window_dim = t2_window_dimension;
	} else if ( mode == MODE_T3 ) {
		if ( order == 1 ) {
			error("Bin intensity is not well-defined for order 1.\n");
			bin_intensity_free(&bin_intensity);
			return(bin_intensity);
		}

		bin_intensity->window_scale = pulse_scale;
		memcpy(&(bin_intensity->window_limits),
				pulse_limits,
				sizeof(limits_t));
		bin_intensity->channel_dim = t3_channel_dimension;
		bin_intensity->window_dim = t3_window_dimension;
	} else {
		error("Invalid mode: %d\n", mode);
		bin_intensity_free(&bin_intensity);
		return(bin_intensity);
	}

	bin_intensity->edges = edges_alloc(bin_intensity->window_limits.bins);

	if ( bin_intensity->edges == NULL ) {
		bin_intensity_free(&bin_intensity);
		return(bin_intensity);
	}

	bin_intensity->counts = (unsigned long long **)malloc(
			sizeof(unsigned long long *)*bin_intensity->channels);

	if ( bin_intensity->counts == NULL ) {
		bin_intensity_free(&bin_intensity);
		return(bin_intensity);
	}

	for ( i = 0; i < bin_intensity->channels; i++ ) {
		bin_intensity->counts[i] = (unsigned long long *)malloc(
			sizeof(unsigned long long)*bin_intensity->window_limits.bins);

		if ( bin_intensity->counts[i] == NULL ) {
			bin_intensity_free(&bin_intensity);
			return(bin_intensity);
		}
	}

	bin_intensity->queue = queue_alloc(sizeof(photon_t), queue_size);

	if ( bin_intensity->queue == NULL ) {
		bin_intensity_free(&bin_intensity);
		return(bin_intensity);
	}

	return(bin_intensity);
}

void bin_intensity_init(bin_intensity_t *bin_intensity,
		int set_start, long long start,
		int set_stop, long long stop) {
	int i;

	bin_intensity->flushing = false;

	bin_intensity->set_start = set_start;
	bin_intensity->start = start;
	bin_intensity->set_stop = set_stop;
	bin_intensity->stop = stop;

	edges_init(bin_intensity->edges,
			&(bin_intensity->window_limits),
			bin_intensity->window_scale,
			false);

	for ( i = 0; i < bin_intensity->channels; i++ ) {
		memset(bin_intensity->counts[i],
				0,
				sizeof(unsigned long long)*bin_intensity->window_limits.bins);
	}

	queue_init(bin_intensity->queue);

	bin_intensity->maximum_delay = (long long)floor(
			bin_intensity->window_limits.upper - 
			bin_intensity->window_limits.lower);
}

void bin_intensity_free(bin_intensity_t **bin_intensity) {
	int i;

	if ( *bin_intensity != NULL ) {
		edges_free(&((*bin_intensity)->edges));

		if ( (*bin_intensity)->counts != NULL ) {
			for ( i = 0; i < (*bin_intensity)->channels; i++ ) {
				free((*bin_intensity)->counts[i]);
			}
		}

		queue_free(&((*bin_intensity)->queue));
		free(*bin_intensity);
		*bin_intensity = NULL;
	}
}

void bin_intensity_increment(bin_intensity_t *bin_intensity) {
	int i;

	unsigned int channel;

	double left;
	double right;

	double left_window;
	double current_window;
	double right_window;

	queue_pop(bin_intensity->queue, &(bin_intensity->photon));

	left_window = (double)bin_intensity->start;
	current_window = (double)bin_intensity->window_dim(
			&(bin_intensity->photon));
	right_window = (double)bin_intensity->stop;

	for ( i = 0; i < bin_intensity->window_limits.bins; i++ ) {
		left = current_window + bin_intensity->edges->bin_edges[i];
		right = current_window + bin_intensity->edges->bin_edges[i+1];

		if ( (left_window <= left && left < right_window) ||
				(left_window <= right && right < right_window) ) {
			channel = bin_intensity->channel_dim(&(bin_intensity->photon));
			bin_intensity->counts[channel][i]++;
		}
	}
}

void bin_intensity_flush(bin_intensity_t *bin_intensity) {
	bin_intensity->flushing = true;

	while ( ! queue_empty(bin_intensity->queue) ) {
		debug("Incrementing.\n");
		bin_intensity_increment(bin_intensity);
	}
}

int bin_intensity_valid_distance(bin_intensity_t *bin_intensity) {
	long long left;
	long long right;

	if ( queue_empty(bin_intensity->queue) ) {
		return(false);
	}

	queue_front(bin_intensity->queue, (void *)&(bin_intensity->photon));
	left = bin_intensity->window_dim(&(bin_intensity->photon));
	right = bin_intensity->stop;

	return( right - left > bin_intensity->maximum_delay );
}

int bin_intensity_push(bin_intensity_t *bin_intensity, photon_t const *photon) {
	int result;
	unsigned int channel = bin_intensity->channel_dim(photon);
	long long window = bin_intensity->window_dim(photon);

	if ( channel >= bin_intensity->channels ) {
		error("Invalid channel: %d (limit %d)\n", channel,
				bin_intensity->channels - 1);
		return(PC_ERROR_INDEX);
	}

	if ( bin_intensity->set_start && window <= bin_intensity->start ) {
		debug("Before window.\n");
		return(PC_SUCCESS);
	} else if ( bin_intensity->set_stop && window > bin_intensity->stop ) {
		debug("After window.\n");
		return(PC_SUCCESS);
	} else {
		debug("In window.\n");
		if ( ! bin_intensity->set_start ) {
			bin_intensity->set_start = true;
			bin_intensity->start = window;
		}

		if ( ! bin_intensity->set_stop ) {
			bin_intensity->stop = window;
		}
	
		result = queue_push(bin_intensity->queue, photon);

		if ( result != PC_SUCCESS ) {
			return(result);
		}

		while ( bin_intensity_valid_distance(bin_intensity) ) {
			bin_intensity_increment(bin_intensity);
		}

		return(PC_SUCCESS);
	}
}

int bin_intensity_fprintf(FILE *stream_out, 
		bin_intensity_t const *bin_intensity) {
	int i;
	int j;

	for ( i = 0; i < bin_intensity->window_limits.bins; i++ ) {
		fprintf(stream_out, "%lf,%lf",
				bin_intensity->edges->bin_edges[i],
				bin_intensity->edges->bin_edges[i+1]);

		for ( j = 0; j < bin_intensity->channels; j++ ) {
			fprintf(stream_out, ",%llu", bin_intensity->counts[j][i]);
		}

		fprintf(stream_out, "\n");
	}

	return( ferror(stream_out) ? PC_ERROR_IO : PC_SUCCESS );
}

int bin_intensity_fprintf_bins(FILE *stream_out,
		bin_intensity_t const *bin_intensity, unsigned int const blanks) {
	return(PC_ERROR_OPTIONS);
}

int bin_intensity_fprintf_counts(FILE *stream_out,
		bin_intensity_t const *bin_intensity) {
	return(PC_ERROR_OPTIONS);
}

int bin_intensity(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	photon_stream_t *photons;
	bin_intensity_t *bin_intensity;

	photons = photon_stream_alloc(options->mode);
	bin_intensity = bin_intensity_alloc(options->mode, options->order,
			options->channels,
			&(options->time_limits), options->time_scale,
			&(options->pulse_limits), options->pulse_scale,
			options->queue_size);

	if ( photons == NULL || bin_intensity == NULL ) {
		error("Could not allocate photon stream or bin intensity.\n");
		result = PC_ERROR_MEM;
	} 

	if ( result == PC_SUCCESS ) {
		photon_stream_init(photons, stream_in);
		photon_stream_set_unwindowed(photons);

		bin_intensity_init(bin_intensity,
				options->set_start, options->start,
				options->set_stop, options->stop);

		debug("Running.\n");
		while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
			debug("Found photon.\n");
			result = bin_intensity_push(bin_intensity, &(photons->photon));

			if ( result != PC_SUCCESS ) {
				if ( result == PC_WINDOW_NEXT ) {
					result = PC_SUCCESS;
					break;
				} else {
					error("Could not add photon to bin_intensity: %d.\n", 
							result);
					break;
				}
			}
		}

		debug("Flushing.\n");
		if ( result == PC_SUCCESS ) {
			bin_intensity_flush(bin_intensity);
			bin_intensity_fprintf(stream_out, bin_intensity);
		}
	}

	photon_stream_free(&photons);
	bin_intensity_free(&bin_intensity);

	return(result);
}
