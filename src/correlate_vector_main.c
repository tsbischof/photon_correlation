#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "files.h"
#include "modes.h"
#include "options.h"
#include "correlate_vector.h"

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options;

	FILE *in_stream = NULL;
	FILE *out_stream = NULL;

	program_options_t program_options = {
		9,
"This program performs the autocorrelation to nth order of a signal with\n"
"float intensity. It outputs the raw correlation and the terms required to\n"
"normalize it.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_CHANNELS, 
			OPT_PRINT_EVERY,
			OPT_TIME, OPT_TIME_SCALE}};

	
	result = parse_options(argc, argv, &options, &program_options);

	result += open_streams(&in_stream, options.in_filename,
			&out_stream, options.out_filename);

	/* Begin the calculation. */
	if ( ! result ) {
		result = correlate_vector(in_stream, out_stream, &options);
	}

	/* Free memory. */
	free_options(&options);
	free_streams(in_stream, out_stream);
	
	return(parse_result(result));
}
