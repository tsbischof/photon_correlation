#include <stdio.h>

#include "error.h"
#include "files.h"
#include "options.h"
#include "intensity_to_t2.h"

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options;

	FILE *stream_in = NULL;
	FILE *stream_out = NULL;

	program_options_t program_options = {
"This program accepts intensity data of the form:\n"
"      time (long long), counts (unsigned int)\n"
"and outputs an equivalent t2 photon stream, tagged on some number\n"
"of channels. By default, all photons are sent to channel 0, but \n"
"if a finite number of channels is specified the photons are \n"
"randomly distributed among the possible channels.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION, 
			OPT_FILE_IN, OPT_FILE_OUT, 
			OPT_BINARY_IN, OPT_BINARY_OUT,
			OPT_SEED, OPT_CHANNELS, OPT_EOF}};

	options.channels = 1;
	result = parse_options(argc, argv, &options, &program_options);

	if ( result == PC_SUCCESS ) {
		result += open_streams(&stream_in, options.filename_in,
				&stream_out, options.filename_out);
	}

	if ( result == PC_SUCCESS ) {
		result = intensity_to_t2(stream_in, stream_out, &options);
	}

	/* Free memory. */
	free_options(&options);
	free_streams(stream_in, stream_out);
	
	return(pc_check(result));
}
