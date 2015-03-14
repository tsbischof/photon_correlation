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
#include "statistics/number_to_channels.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program accepts a stream of t3 photons and return the photons, with \n"
"channels remapped to indicate the number of photons seen in each pulse.\n"
"\n"
"Note that photons are restricted to arrival on distinct channels; only\n"
"the first photon on a channel during a given pulse will be used.\n"
"\n"
"Photons in pulse            Channels mapped to \n"
"         1                     0\n"
"         2                     1, 2\n"
"         3                     3, 4, 5\n"
"        ...                    .... \n"
"\n"
"The first index follows:\n"
" f(n) = (n - 2)(n - 1)/2\n"
"\n"
"To determine the order in the pulse and the number of photons in that pulse:\n"
"number in pulse = (int)floor((1+sqrt(1+8*channel))/2)\n"
"order in pulse = channel - number in pulse + 1\n"
"\n"
"If the option --correlate-successive is used, the photons' arrival times\n"
"are correlated in sucession, rather than relative to the excitation pulse.\n"
"For example:\n"
"    0,1,12; 1,1,14   ---->     1,1,12; 2,1,2\n"
"This way, the lifetimes may be calculated for successive events to determine\n"
"whether the kinetics are successive or parallel.",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_QUEUE_SIZE, 
			OPT_CORRELATE_SUCCESSIVE,
			OPT_EOF}};

	return(run(&program_options, number_to_channels, argc, argv));
}
