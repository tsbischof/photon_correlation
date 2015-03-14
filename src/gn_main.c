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

#include "options.h"
#include "run.h"
#include "gn.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program performs the calculations needed to calculate the correlation\n"
"of a stream of photons, including those needed to normalize the result.\n"
"It does not perform the normalization, but instead produces the results\n"
"necessary to do so.\n"
"\n"
"For a given input file, some number of files will be output:\n"
" 1. intensity data\n"
"    a. *.count-all: The total number of counts found on all channels, useful\n"
"       for approximate normalization. By default, this is the output.\n"
"    b. *.bin_intensity: The exact values needed to normalize each histogram\n"
"       bin. This is more expensive to calculate than .count-all, so only\n"
"       use this if the exact result is desired.\n"
" 2. Correlation data\n"
"    *.gn: The n is the appropriate value for the given correlation. This\n"
"    file contains the histogrammed correlation events, and is the non-\n"
"    normalized correlation.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION, 
			OPT_FILE_IN, OPT_FILE_OUT, 
			OPT_MODE, OPT_CHANNELS, OPT_ORDER, 
			OPT_QUEUE_SIZE, 
			OPT_WINDOW_WIDTH,
			OPT_TIME, OPT_PULSE,
			OPT_BIN_WIDTH,
			OPT_PRINT_EVERY,
			OPT_EOF}};

	return(gn_run(&program_options, argc, argv));
}


