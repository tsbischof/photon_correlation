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
#include "flid.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"In the study of nanocrystal photoluminescence intermittency, there is often\n"
"a correlation between emission intensity and lifetime. Quantifying this\n"
"behavior can be done in several ways, including:\n"
"1. Measurement of the lifetime for all photons emitted during periods\n"
"   of equivalent intensity.\n"
"2. Estimation of the instantaneous lifetime and intensity.\n"
"\n"
"This second method is used here, to calculate what is known as the\n"
"Fluorescence Lifetime Intensity Diagram (FLID).\n"
"\n"
"The FLID is calculated by first dividing the photon stream into windows\n"
"of equivalent time (the same number of pulses, as lifetimes require t3-mode\n"
"data), then determining the mean arrival time for all photons in each \n"
"window. An alternative approach is to calculate the lifetime histogram\n"
"for each window, then to fit the data to an exponential form and report the\n"
"optimal time constant. For this program, the mean arrival time is determined\n"
"directly by counting the number of photons which arrive in the window and \n"
"their cumulative arrival time, which are then divided to determine the mean.\n"
"\n"
"The cumulative arrival time is internally handled as a 64-bit unsigned\n"
"integer, so if you are seeing odd results check to make sure that you are\n"
"not including too many photons per window. In picoseconds, this would be\n"
"10^8 photons each arriving 1 second after an excitation pulse (10^12 ps).\n"
"\n"
"The output format is a csv file, where the first two rows specify the lower\n"
"and upper limits for the mean arrival time histogram, and the first two\n"
"columns specify the lower and upper limits for the intensity histogram.\n"
"The second row of the second column contains the window width.\n"
"For example, with arrival times spaced by 10ns, a window width of 100, and\n"
"and intensity bins spaced by 10 photons per window, the output file might\n"
"look like this:\n"
"   ,    ,    ,     0, 10000, 20000, ...\n"
"   ,    ,    , 10000, 20000, 30000, ...\n"
"  0,  10, 100,   xxx,   xxx,   xxx, ...\n"
" 10,  20, 100,   xxx,   xxx,   xxx, ...\n"
" ...\n"
"(whitespace added for clarity, xxx represent integer numbers of events which\n"
"were found with the given intensity and mean arrival time).\n"
"\n"
"Windows with no photons are discarded, so signal at zero arrival time and \n"
"near-zero intensity is real. A uniform background will bias the measurement\n"
"toward half the repetition period, so at low intensities the FLID will tend\n"
"to exhibit a peak near this value.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION, 
			OPT_FILE_IN, OPT_FILE_OUT, 
			OPT_WINDOW_WIDTH, OPT_TIME, OPT_INTENSITY,
			OPT_EOF}};

	return(run(&program_options, flid, argc, argv));
}
