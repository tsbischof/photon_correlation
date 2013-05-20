#include "run.h"
#include "options.h"
#include "photon/temper.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program performs some basic manipulations of tttr data. This includes\n"
"suppression of channel data (removal of all such photons from a stream)\n"
"and addition of a constant offset to particular channels.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_CHANNELS,
			OPT_TIME_OFFSETS, OPT_PULSE_OFFSETS,
			OPT_SUPPRESS, OPT_QUEUE_SIZE, 
			OPT_FILTER_AFTERPULSING, OPT_EOF}};

	return(run(&program_options, photon_temper, argc, argv));
}
