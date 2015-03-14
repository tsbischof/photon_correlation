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

#include "photon_intensity_correlate.h"
#include "run.h"
#include "options.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program performs an intensity correlation of photon arrival time data.\n"
"The algorithm used mimics that of standard multi-tau hardware correlators,\n"
"which perform linear-binned intensity correlations at logarithmically-\n"
"increasing bin widths. For example, with three registers and a binning of\n"
"three, the effective bins look like:\n"
" |0|1|2|\n"
" |     |  5  |  8  | \n"
" |                 |       13        |       22        |\n"
"In this picture, each row forms some number of correlation bins by a normal\n"
"linear correlation of the blank bin with the others. By updating the signal\n"
"by pushing existing values into the larger bins as the small bins are\n"
"filled, we obtain an effective logarithmic scaling, up to some number of\n"
"rescales (the depth). This way, each time bin for the correlation is only\n"
"sampled as often as necessary, ensuring that reasonable signal-to-noise\n"
"can be obtained at all time scales without requiring significant duplication\n"
"of effort.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_CHANNELS, OPT_ORDER,
			OPT_MODE, 
			OPT_START, OPT_STOP,
			OPT_BIN_WIDTH,
			OPT_TIME_SCALE,
			OPT_BINNING, OPT_REGISTERS, OPT_DEPTH,
			OPT_EOF}};

	return(run(&program_options, photon_intensity_correlate_dispatch, 
			argc, argv));
}
