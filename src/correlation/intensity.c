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

#include <math.h>

#include "intensity.h"
#include "../statistics/intensity.h"
#include "../modes.h"
#include "../error.h"
#include "multi_tau.h"

int intensity_correlate_g2_log(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	multi_tau_g2cn_t *mt;
	intensity_photon_t *intensity;

	mt = multi_tau_g2cn_alloc(options->binning, options->registers, 
			options->depth, options->channels, 1);
	intensity = intensity_photon_alloc(options->channels, MODE_T2);

	if ( mt == NULL || intensity == NULL ) {
		error("Could not allocate correlator or intensity stream.\n");
		result = PC_ERROR_MEM;
	}

	if ( result == PC_SUCCESS ) {
		intensity_photon_init_stream(intensity, stream_in);
		multi_tau_g2cn_init(mt);

		while ( intensity_photon_next(intensity) == PC_SUCCESS ) {
			multi_tau_g2cn_push(mt, intensity->counts);
		}
	}

	if ( result == PC_SUCCESS ) {
		multi_tau_g2cn_fprintf(stream_out, mt);
	}

	multi_tau_g2cn_free(&mt);
	intensity_photon_free(&intensity);

	return(PC_SUCCESS);
}

int intensity_correlate_dispatch(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	if ( options->order == 2 && options->time_scale == SCALE_LOG ) {
		return(intensity_correlate_g2_log(stream_in, stream_out, options));
	} else {
		error("Correlation mode not supported: order %u and scale %d\n",
				options->order, options->time_scale);
		return(PC_ERROR_OPTIONS);
	}
}
