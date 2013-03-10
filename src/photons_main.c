#include "run.h"
#include "options.h"
#include "photon/photons.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program reads and writes photons. It is largely meant as a debugging\n"
"and profiling tool, since other programs will call the underlying routines\n"
"themselves to read data as needed.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_CONVERT,
			OPT_REPETITION_TIME, OPT_EOF}};

	return(run(&program_options, photons, argc, argv));
}
