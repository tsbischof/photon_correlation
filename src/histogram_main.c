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

#include "run.h"
#include "options.h"
#include "histogram.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"Histogram collects a set of photon correlation events and counts them into\n"
"bins defined by their relative time delays. The histograms are also\n"
"separated by the identities of each channel in the correlation, such that\n"
"all cross-correlations can be recovered. The output is similar to the \n"
"correlation events, except that the delay limits are now given:\n"
"   (t2, order 2)\n"
"   channel 0, channel 1, t1-t0 lower, t1-t0 upper, counts\n"
"Extension to higher orders creates more channel-delay pairs, and for t3 mode\n"
"the pair becomes a channel-pulse-time tuple.\n"
"\n"
"An order 1 correlation of t3 data is possible, since this is already \n"
"implicitly a correlation of a sync source and a photon stream. This is\n"
"useful for calculating a photoluminescence lifetime, for example.\n",
		{OPT_HELP, OPT_VERBOSE, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_CHANNELS, OPT_ORDER,
			OPT_TIME, OPT_PULSE, OPT_TIME_SCALE, OPT_PULSE_SCALE, OPT_EOF}};

	return(run(&program_options, histogram_dispatch, argc, argv));
}
