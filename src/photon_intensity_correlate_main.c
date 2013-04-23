#include "photon_intensity_correlate.h"
#include "run.h"
#include "options.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
		"",

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
