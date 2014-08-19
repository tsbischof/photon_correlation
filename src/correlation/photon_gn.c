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

#include "photon_gn.h"
#include <math.h>
#include "../error.h"
#include "../correlation/photon.h"
/* 
 * For correlation we typically need to join several operations together.
 * At minimum, we must send the photons to the correlator and the correlations
 * to the histogrammer, but for diagnostic purposes it is often desirable
 * to send photons to an intensity measurement or other routine. As such, this
 * procedure is meant to push photons to all of the different measurements
 * desired, such that a single pass through the stream is sufficient for 
 * performing all calculations. This in principle enables real-time processing
 * but in practice just makes the whole act of processing simpler.
 */

photon_gn_t *photon_gn_alloc(int const mode, int const order, 
		int const channels, size_t const queue_size,
		limits_t const *time_limits, limits_t const *pulse_limits) {
	photon_gn_t *gn = NULL;
	long long min_time_distance, max_time_distance;
	long long min_pulse_distance, max_pulse_distance;

	gn = (photon_gn_t *)malloc(sizeof(photon_gn_t));

	if ( gn == NULL ) {
		return(gn);
	}

	min_time_distance = time_limits->lower < 0 ? 0 :
			(long long)floor(time_limits->lower);
	max_time_distance = (long long)ceil(
			max(fabs(time_limits->lower), fabs(time_limits->upper)));
	min_pulse_distance = pulse_limits->lower < 0 ? 0 :
			(long long)floor(pulse_limits->lower);
	max_pulse_distance = (long long)ceil(
			max(fabs(pulse_limits->lower), fabs(pulse_limits->upper)));

	debug("Limits: time: (%lld, %lld); pulse: (%lld, %lld)\n",
			min_time_distance, max_time_distance,
			min_pulse_distance, max_pulse_distance);

	gn->correlator = correlator_alloc(mode, order, 
			queue_size, false,
			min_time_distance, max_time_distance,
			min_pulse_distance, max_pulse_distance);
	gn->histogram = histogram_gn_alloc(mode, order, channels,
			SCALE_LINEAR, time_limits,
			SCALE_LINEAR, pulse_limits);

	if ( gn->correlator == NULL || gn->histogram == NULL ) {
		photon_gn_free(&gn);
		return(gn);
	}

	return(gn);
}

void photon_gn_init(photon_gn_t *gn) {
	correlator_init(gn->correlator);
	histogram_gn_init(gn->histogram);
}

int photon_gn_push(photon_gn_t *gn, photon_t const *photon) {
	int result = correlator_push(gn->correlator, photon);

	while ( result == PC_SUCCESS &&
				correlator_next(gn->correlator) == PC_SUCCESS ) {
		printf("here\n");
		histogram_gn_increment(gn->histogram,
				gn->correlator->correlation);
	}

	return(result);
}

int photon_gn_flush(photon_gn_t *gn) {
	int result = PC_SUCCESS;

	correlator_flush(gn->correlator);

	while ( result == PC_SUCCESS && 
			correlator_next(gn->correlator) == PC_SUCCESS ) {
		histogram_gn_increment(gn->histogram,
				gn->correlator->correlation);
	}

	return(result);
}

int photon_gn_fprintf(FILE *stream_out, photon_gn_t const *gn) {
	return(histogram_gn_fprintf(stream_out, gn->histogram));
}

int photon_gn_fprintf_bins(FILE *stream_out, photon_gn_t const *gn,
		unsigned int const blanks) {
	return(histogram_gn_fprintf_bins(stream_out, gn->histogram, blanks));
}

int photon_gn_fprintf_counts(FILE *stream_out, photon_gn_t const *gn) {
	return(histogram_gn_fprintf_counts(stream_out, gn->histogram));
}

void photon_gn_free(photon_gn_t **gn) {
	if  ( *gn != NULL ) {
		correlator_free(&((*gn)->correlator));
		histogram_gn_free(&((*gn)->histogram));

		free(*gn);
		*gn = NULL;
	}
}
