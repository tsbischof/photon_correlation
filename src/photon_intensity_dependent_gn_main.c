/*
Copyright (c) 2011-2014, Thomas Bischof
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. Neither the name of the Massachusetts Institute of Technology nor the 
   names of its contributors may be used to endorse or promote products 
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/

#include "options.h"
#include "run.h"
#include "intensity_dependent_gn.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"In some analysis of emission data, it is useful to correlate some emission\n"
"behavior with the instanantaneous intensity of emission. This program is \n"
"designed to calculate a photon correlation function for fixed intervals of\n"
"time, and to accumulate the correlation function of periods of equivalent\n"
"emission intensity.\n"
"\n"
"Learn more about the details for the correlation function, see photon_gn.\n"
"\n"
"To specify the intensity definition, use --intensity. This groups windows\n"
"of time based on the number of photons which arrived in the window.\n"
"\n"
"The output is similar to that of photon_gn with a window width (calculation\n"
"of the correlation every fixed unit of time). It is a csv file where the \n"
"first few rows represent the defintion of the correlation function \n"
"(channels, time and pulse bins, etc.) and each row represents the counts \n"
"found in a particular correlation. In this case, each row represents the \n"
"collected correlation for a given range of intensities, along with the \n"
"number of time windows which contributed to that correlation. For a g1 (t3 \n"
"mode used to measure emission lifetime), this looks something like\n"
"     ,      ,     ,    0,    0,    0, ... \n"
"     ,      ,     ,    0, 1000, 2000, ... \n"
"     ,      ,     , 1000, 2000, 3000, ... \n"
"    0, 0.001,  xxx,  yyy,  yyy,  yyy, ... \n"
"0.001, 0.002,  xxx,  yyy,  yyy,  yyy, ... \n"
"where the xxx are integer counts of the number of time windows and yyy are \n"
"counts associated with the correlation function. If fixed intensity bins are\n"
"insufficient, use photon_gn with a window width specified, and use that raw\n"
"data to perform the accumulation.",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION, 
			OPT_FILE_IN, OPT_FILE_OUT, 
			OPT_MODE, OPT_CHANNELS, OPT_ORDER,
			OPT_QUEUE_SIZE,
			OPT_WINDOW_WIDTH, 
			OPT_TIME, OPT_PULSE,
			OPT_BIN_WIDTH,
			OPT_EOF}};

	return(run(&program_options, intensity_dependent_gn, argc, argv));
}
