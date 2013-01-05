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
#include "photons.h"
#include "options.h"

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options;

	FILE *stream_in = NULL;
	FILE *stream_out = NULL;

	program_options_t program_options = {
		7,
"This program reads and writes photons. It is largely meant as a debugging\n"
"and profiling tool, since other programs will call the underlying routines\n"
"themselves to read data as needed.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_USE_VOID}};
	
	result = parse_options(argc, argv, &options, &program_options);

	result += open_streams(&stream_in, options.filename_in,
			&stream_out, options.filename_out);

	/* Begin the calculation. */
	if ( ! result ) {
		result = photons_dispatch(stream_in, stream_out, &options);
	}

	/* Free memory. */
	free_options(&options);
	free_streams(stream_in, stream_out);
	
	return(pc_check(result));
}
