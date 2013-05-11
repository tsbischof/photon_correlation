#include "options.h"
#include "run.h"
#include "statistics/number_to_channels.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program accepts a stream of t3 photons and return the photons, with \n"
"channels remapped to indicate the number of photons seen in each pulse."
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
