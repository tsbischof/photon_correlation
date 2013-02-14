#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "error.h"
#include "files.h"
#include "correlate_photon.h"
#include "options.h"

int main(int argc, char *argv[]) {
	int result = PC_SUCCESS;

	options_t options = {};

	FILE *stream_in = NULL;
	FILE *stream_out = NULL;

	program_options_t program_options = {
"This program reads and writes correlations of photons.\n"
" It is largely meant as a debugging and profiling tool, since other programs\n"
"will call the underlying routines themselves to read data as needed.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_ORDER, OPT_EOF}};
	
	result = parse_options(argc, argv, &options, &program_options);

	result += open_streams(&stream_in, options.filename_in,
			&stream_out, options.filename_out);

	/* Begin the calculation. */
	if ( ! result ) {
		result = correlations_echo(stream_in, stream_out, &options);
	}

	/* Free memory. */
	free_options(&options);
	free_streams(stream_in, stream_out);
	
	return(pc_check(result));
}
