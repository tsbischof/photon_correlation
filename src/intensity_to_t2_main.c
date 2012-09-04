#include <stdio.h>

#include "error.h"
#include "files.h"
#include "options.h"
#include "intensity_to_t2.h"

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options;

	FILE *in_stream = NULL;
	FILE *out_stream = NULL;

	program_options_t program_options = {
		6,
"This program accepts intensity data of the form:\n"
"      time (long long), counts (unsigned int)\n"
"and outputs an equivalent t2 photon stream, tagged on some number\n"
"of channels. By default, all photons are sent to channel 0, but \n"
"if a finite number of channels is specified the photons are \n"
"randomly distributed among the possible channels.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT, OPT_CHANNELS}};

	options.channels = 1;
	result = parse_options(argc, argv, &options, &program_options);

	result += open_streams(&in_stream, options.in_filename,
			&out_stream, options.out_filename);

	/* Begin the calculation. */
	if ( ! result ) {
		result = intensity_to_t2(in_stream, out_stream, &options);
	}

	/* Free memory. */
	free_options(&options);
	free_streams(in_stream, out_stream);
	
	return(parse_result(result));
}
