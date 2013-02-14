#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "files.h"
#include "options.h"
#include "permutations.h"

int main(int argc, char *argv[]) {
	int result = PC_SUCCESS;

	options_t options = {};

	FILE *stream_in = NULL;
	FILE *stream_out = NULL;

	program_options_t program_options = {
"Constructs and prints combinations, for testing purposes.",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_OUT, 
			OPT_ORDER, OPT_EOF}};
	
	result = parse_options(argc, argv, &options, &program_options);

	if ( result == PC_SUCCESS ) {
		result = open_streams(&stream_in, options.filename_in,
				&stream_out, options.filename_out);
	}

	/* Begin the calculation. */
	if ( result == PC_SUCCESS ) {
		result = combinations_dispatch(stream_in, stream_out, &options);
	}

	/* Free memory. */
	free_options(&options);
	free_streams(stream_in, stream_out);
	
	return(pc_check(result));
}
