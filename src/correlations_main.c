#include "run.h"
#include "correlation/photon.h"
#include "options.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program reads and writes correlations of photons.\n"
" It is largely meant as a debugging and profiling tool, since other programs\n"
"will call the underlying routines themselves to read data as needed.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_ORDER, OPT_EOF}};
	return(run(&program_options, correlations_echo, argc, argv));
}
