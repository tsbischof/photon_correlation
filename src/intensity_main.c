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

#include "run.h"
#include "options.h"
#include "statistics/intensity.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"Given a stream of TTTR data formatted like the output of picoquant, this\n"
"program calculates the number of photons arriving on any number of detection\n"
"channels, divided into some number of subsets of integration time.\n"
"\n"
"For example, to calculate the intensity of a stream of t2 photons from a\n"
"measurement on a Picoharp, collected in 50ms bins:\n"
"    intensity --bin-width 50000000000 --mode t2 --channels 2\n"
"\n"
"The output format is:\n"
"    time start, time stop, channel 0 counts, channel 1 counts, ...\n"
"\n"
"Because the final photon may not arrive at the end of a time bin, the final\n"
"bin ends at the arrival time of the last photon, permitting post-processing\n"
"to determine whether the effect of that edge is significant.\n"
"\n"
"As an alternative to time bins, all of the photons can be counted by passing\n"
"the flag --count-all. This is useful for normalizing a signal.\n",
		{OPT_HELP, OPT_VERBOSE, OPT_VERSION,
			OPT_PRINT_EVERY,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_START, OPT_STOP,
			OPT_MODE, OPT_CHANNELS,
			OPT_BIN_WIDTH, OPT_COUNT_ALL,
			OPT_EOF}};

	return(run(&program_options, intensity_photon, argc, argv));
}
