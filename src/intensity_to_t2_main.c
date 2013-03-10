#include "run.h"
#include "options.h"
#include "intensity_to_t2.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program accepts intensity data of the form:\n"
"      time (long long), counts (unsigned int)\n"
"and outputs an equivalent t2 photon stream, tagged on some number\n"
"of channels. By default, all photons are sent to channel 0, but \n"
"if a finite number of channels is specified the photons are \n"
"randomly distributed among the possible channels.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION, 
			OPT_FILE_IN, OPT_FILE_OUT, 
			OPT_SEED, OPT_CHANNELS, OPT_EOF}};

	return(run(&program_options, intensity_to_t2, argc, argv));
}
