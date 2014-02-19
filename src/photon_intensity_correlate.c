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

#include "photon_intensity_correlate.h"

#include "correlation/multi_tau.h"
#include "statistics/intensity.h"
#include "modes.h"
#include "photon/stream.h"
#include "error.h"

int photon_intensity_correlate_g2_log(FILE *stream_in, FILE *stream_out,
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	long long bin_width;
	intensity_photon_t *intensity;
	photon_stream_t *photon_stream;
	multi_tau_g2cn_t *mt;

	debug("Allocating intensity, photon stream.\n");
	bin_width = options->bin_width;
	if ( bin_width == 0 ) {
		if ( options->mode == MODE_T2 ) {
			bin_width = 10000000;
		} else {
			bin_width = 100;
		}
	}

	intensity = intensity_photon_alloc(options->channels, options->mode);
	photon_stream = photon_stream_alloc(options->mode);
	mt = multi_tau_g2cn_alloc(options->binning, options->registers,
			options->depth, options->channels, bin_width);


	if ( intensity == NULL || photon_stream == NULL || mt == NULL ) {
		error("Could not allocate intensity or photon stream.\n");
		result = PC_ERROR_MEM;
	} 

	if ( result == PC_SUCCESS ) {
		debug("Initializing.\n");
		intensity_photon_init(intensity,
				false,
				bin_width,
				options->set_start, options->start,
				options->set_stop, options->stop);

		photon_stream_init(photon_stream, stream_in);
		multi_tau_g2cn_init(mt);

		while ( photon_stream_next_photon(photon_stream) == PC_SUCCESS ) {
			intensity_photon_push(intensity, photon_stream->photon);
	
			while ( intensity_photon_next(intensity) == PC_SUCCESS ) {
				multi_tau_g2cn_push(mt, intensity->counts);
			}
		}
	
		intensity_photon_flush(intensity);
		while ( intensity_photon_next(intensity) == PC_SUCCESS ) {
			multi_tau_g2cn_push(mt, intensity->counts);
		}

		multi_tau_g2cn_fprintf(stream_out, mt);
	}

	debug("Cleaning up.\n");
	intensity_photon_free(&intensity);
	photon_stream_free(&photon_stream);
	multi_tau_g2cn_free(&mt);
	return(PC_SUCCESS);
}
	
int photon_intensity_correlate_dispatch(FILE *stream_in, FILE *stream_out,
		pc_options_t const *options) {
	if ( options->order == 2 ) {
		return(photon_intensity_correlate_g2_log(stream_in, stream_out, 
				options));
	} else {
		error("Unsupported mode: order %d\n", options->order);
		return(PC_ERROR_OPTIONS);
	}
}
