#include "statistics/number.h"
#include "run.h"
#include "options.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program accepts t3 photons and records the number of times 0, 1, ... n\n"
"photons arrive from individual pulses. After reading the stream, it outputs\n"
"these counts in the format: \n"
"      number seen, times seen\n"
"\n"
"By default, the maximum number is set much higher than the number of\n"
"channels, which covers the case where detector dead time is long relative\n"
"to the time span between pulses. \n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_CHANNELS,
			OPT_EOF}};

	return(run(&program_options, photon_number, argc, argv));
}
