/*
 * Copyright (c) 2011-2015, Thomas Bischof
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

#include "threshold.h"
#include "stream.h"
#include "../error.h"
#include "../modes.h"

/*
This code is designed to take a stream of photons as input, and output the 
photons found in time windows with at least some number of photons. The 
following Python code implements the algorithm:
--------------------------------------------------------------------------------
import photon_correlation_examples as pce

class Window:
    def __init__(self, width, start=0):
        self.start = start
        self.width = width

    def __str__(self):
        return("[{},{})".format(self.start, self.width))

    def __contains__(self, time):
        return(time in range(self.start, self.start+self.width))

    def before(self, time):
        return(time < self.start)

    def after(self, time):
        return(time >= (self.start + self.width))

    def next(self):
        self.start += self.width

class Threshold:
    def __init__(self, window_width, threshold):
        self.queue = list()
        self.threshold = threshold
        self.flushing = False
        self.window = Window(window_width)

    def __iter__(self):
        if len(self.queue) > 0:
            if self.flushing:
                if len(self.queue) < self.threshold:
                    self.queue.clear()
                else:
                    photons = self.queue[:]
                    self.queue.clear()
                    for photon in photons:
                        yield(photon)
            else:
                if self.window.after(self.queue[-1].window_dimension):
##                    print(self.window.start, self.queue)
                    if len(self.queue[:-1]) < self.threshold:
                        photons = list()
                    else:
                        photons = self.queue[:-1]

                    self.queue[:-1] = []

                    for photon in photons:
                        yield(photon)

                    while not self.queue[-1].window_dimension in self.window:
                        self.window.next()
                        
        raise(StopIteration)

    def flush(self):
        self.flushing = True

    def push(self, photon):
        if self.window.before(photon.window_dimension):
##            print("Rejecting", photon, "from", self.window)
            pass
        else:
            self.queue.append(photon)

threshold = Threshold(2, 3)

for photon in pce.PhotonStream("/home/tsbischof/src/photon_correlation"
                               "/sample_data/t3.txt", mode="t3"):
    threshold.push(photon)
    
    for photon in threshold:
        print(photon)

threshold.flush()
for photon in threshold:
    print(photon)
--------------------------------------------------------------------------------
The method is complicated in C by the fact that the paradigm in this package is
that of an iterator, and thus we have to keep track of state: the simple
while loops must be explicitly tracked.
 */


photon_threshold_t *photon_threshold_alloc(int const mode, 
		size_t const queue_size) {
	photon_threshold_t *pt = NULL;

	pt = (photon_threshold_t *)malloc(sizeof(photon_threshold_t));

	if ( pt == NULL ) {
		return(pt);
	}

	pt->queue = photon_queue_alloc(mode, queue_size);

	if ( pt->queue == NULL ) {
		photon_threshold_free(&pt);
	}

	if ( pt != NULL ) {
		if ( mode == MODE_T2 ) {
				pt->window_dim = t2_window_dimension;
		} else if ( mode == MODE_T3 ){
			pt->window_dim = t3_window_dimension;
		} else {
			error("Unknown mode: %d\n", mode);
			photon_threshold_free(&pt);
		}
	}

	return(pt);
}

void photon_threshold_init(photon_threshold_t *pt, 
		long long const window_width, long long const threshold,
		int const set_lower_bound, long long const lower_bound,
		int const set_upper_bound, long long const upper_bound) {
	pt->flushing = false;
	pt->yield_window = false;
	photon_window_init(&(pt->window), window_width, 
			set_lower_bound, lower_bound,
			set_upper_bound, upper_bound);
	photon_queue_init(pt->queue);
	pt->threshold = threshold;
}

int photon_threshold_push(photon_threshold_t *pt, photon_t const *photon) {
	int status = photon_window_contains(&(pt->window), pt->window_dim(photon));

	pt->yield_window = false;

	if ( status == PC_RECORD_BEFORE_WINDOW ){
		debug("Before window.\n");
		return(PC_SUCCESS);
	} else {
		return(photon_queue_push(pt->queue, photon));
	}
}

int photon_threshold_next(photon_threshold_t *pt, photon_t *photon) {
	int status = PC_SUCCESS;
	photon_t *front;
	photon_t *back;
	photon_t storage;

	if ( ! photon_queue_empty(pt->queue) ) {
		if ( pt->flushing ) {
			debug("Flushing.\n");
			if ( pt->yield_window || 
					photon_queue_size(pt->queue) >= pt->threshold ) {
				pt->yield_window = true;
				photon_queue_pop(pt->queue, photon);
				return(PC_SUCCESS);
			} else {
				pt->yield_window = false;
				photon_queue_init(pt->queue);
				return(EOF);
			}
		} else {
			photon_queue_back(pt->queue, &back);
			status = photon_window_contains(&(pt->window), 
					pt->window_dim(back));
			if ( status == PC_RECORD_AFTER_WINDOW ) {
				debug("Queue size: %zu\n", photon_queue_size(pt->queue));
				if ( pt->yield_window || 
						(photon_queue_size(pt->queue)-1) >= pt->threshold ) {

/* Need one more photon to emit, since one of the photons in the queue is in
 * the next window.
 */
					photon_queue_pop(pt->queue, photon);
					if ( photon_queue_size(pt->queue) == 1 ) {
						while ( status == PC_RECORD_AFTER_WINDOW ) {
//							debug("next window.\n");
							photon_window_next(&(pt->window));
//							debug("check status\n");
							status = photon_window_contains(&(pt->window),
									pt->window_dim(back));
						}
						debug("in the correct wnidow.\n");
						pt->yield_window = false;
					} else {
						pt->yield_window = true;
					}

					return(PC_SUCCESS);
				} else {
					debug("Not yielding.\n");
					pt->yield_window = false;
					photon_queue_back_copy(pt->queue, &storage);
					photon_queue_init(pt->queue);
					photon_queue_push(pt->queue, &storage);
					while ( status == PC_RECORD_AFTER_WINDOW ) {
						photon_window_next(&(pt->window));
						status = photon_window_contains(&(pt->window),
								pt->window_dim(back));
					}
					return(EOF);
				}
			} else {
				debug("Not yet finished with the window.\n");
				pt->yield_window = false;
				return(EOF);
			}
		}
	} else {
		return(EOF);
	}
}

void photon_threshold_flush(photon_threshold_t *pt) {
	pt->flushing = true;

	if ( photon_queue_size(pt->queue) < pt->threshold ) {
		photon_queue_init(pt->queue);
	}
}

void photon_threshold_free(photon_threshold_t **pt) {
	if ( *pt != NULL ) {
		photon_queue_free(&((*pt)->queue));
		free(*pt);
		*pt = NULL;
	}
}

int photon_threshold(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	photon_stream_t *photons;
	photon_threshold_t *pt;
	photon_t photon;

	debug("Allocating memory\n");
	photons = photon_stream_alloc(options->mode);
	pt = photon_threshold_alloc(options->mode, options->queue_size);

	if ( photons == NULL || pt == NULL ) {
		error("Could not allocate memory.\n");
		result = PC_ERROR_MEM;
	}

	photon_stream_init(photons, stream_in);
	photon_threshold_init(pt, options->window_width, options->threshold,
			options->set_start, options->start,
			options->set_stop, options->stop);

	if ( result == PC_SUCCESS ) {
		debug("Starting stream\n");
		while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
			photon_threshold_push(pt, &(photons->photon));

			while ( photon_threshold_next(pt, &photon) == PC_SUCCESS ) {
				photons->photon_print(stream_out, &photon);
			}
		}

		photon_threshold_flush(pt);
		while ( photon_threshold_next(pt, &photon) == PC_SUCCESS ) {
			photons->photon_print(stream_out, &photon);
		}
	}

	photon_stream_free(&photons);
	photon_threshold_free(&pt);

	return(result);
}
