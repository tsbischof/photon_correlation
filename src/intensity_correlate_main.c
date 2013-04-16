#include "correlation/intensity.h"
#include "run.h"
#include "options.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
		"",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_ORDER,
			OPT_QUEUE_SIZE,
			OPT_TIME, OPT_TIME_SCALE,
			OPT_EOF}};

	return(run(&program_options, intensity_correlate, argc, argv));
}
