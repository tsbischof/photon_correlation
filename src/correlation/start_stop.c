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

#include "start_stop.h"
#include "photon.h"
#include "../photon/t2.h"
#include "../error.h"
#include "../modes.h"

/* The start-stop mode of correlation assumes that one channel is marked as the
 * starting channel, and another as the stopping channel. Any photon arriving
 * on the start channel replaces and existing photon, and any photon arriving
 * on the stop channel either produces a correlation (if photon on start 
 * channel) or nothing (if no photon)
 */
int correlate_start_stop(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int valid = 0;
	t2_t t2;
	t2_next_t next = t2_fscanf;
	correlation_print_t print = t2_correlation_fprintf;

	unsigned long long record_number = 0;

	correlation_t *correlation;

	if ( options->mode != MODE_T2 ) {
		error("Invalid mode for start-stop correlation: %d\n", options->mode);
		return(PC_ERROR_MODE);
	}

	correlation = correlation_alloc(options->mode, options->order);

	if ( correlation == NULL ) {
		return(PC_ERROR_MEM);
	}

	correlation_init(correlation);

	debug("Correlating in start-stop mode.\n");

	while ( next(stream_in, &t2) == PC_SUCCESS ) {
		if ( t2.channel == 0 ) {
			valid = 1;
			correlation_set_index(correlation, 0, &t2);
		} else if ( valid && t2.channel == 1 ) {
			record_number++;
			pc_status_print("correlate", record_number, options);

			correlation_set_index(correlation, 1, &t2);
			t2_correlate(correlation);
			print(stream_out, correlation);
			valid = 0;
		} else if ( t2.channel > 2 ) {
			warn("Start-stop mode only works for 2 channels. "
					"Channel index %d found.\n", t2.channel);
		}
	}

	return(0);
}
