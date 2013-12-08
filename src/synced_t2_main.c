#include "options.h"
#include "run.h"
#include "photon/synced_t2.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program is meant to transform a stream of t2 photons into an equivalent\n"
"stream of t3 photons, where one of the t2 channels is treated as a dedicated\n"
"synchronization channel. There are two main uses for this:\n"
"    1. To treat start-stop mode data as lifetimes.\n"
"    2. To get around the resolution and data rate limitations of t3 hardware\n"
"\n"
"To do this, you must specify the sync channel, and if appropriate the sync\n"
"divider used.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION, 
			OPT_FILE_IN, OPT_FILE_OUT, 
			OPT_SYNC_CHANNEL, OPT_SYNC_DIVIDER,
			OPT_QUEUE_SIZE, 
			OPT_EOF}};

	return(run(&program_options, synced_t2_dispatch, argc, argv));
}


