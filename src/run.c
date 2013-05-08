#include "run.h"
#include "error.h"
#include "files.h"

/*
 * Most programs follow a common routine for processing:
 * 1. Check options. 
 * 2. Initialize I/O
 * 3. Run dispatch to process data.
 * 4. Clean up.
 * 
 * This procedure implements this process.
 */

int run(program_options_t *program_options, dispatch_t const dispatch,
		int const argc, char * const *argv) {
	int result = PC_SUCCESS;
	FILE *stream_in = NULL;
	FILE *stream_out = NULL;
	pc_options_t *options = pc_options_alloc();

	if ( options == NULL ) {
		error("Could not allocate options.\n");
		return(PC_ERROR_MEM);
	}

	pc_options_init(options, program_options);
	result = pc_options_parse(options, argc, argv);

	if ( result != PC_SUCCESS || ! pc_options_valid(options)) {
		if ( options->usage ) {
			pc_options_usage(options, argc, argv);
			result = PC_USAGE;
		} else if ( options->version ) {
			pc_options_version(options, argc, argv);
			result = PC_VERSION;
		} else {
			debug("Invalid options.\n");
			result = PC_ERROR_OPTIONS;
		}
	}

	if ( result == PC_SUCCESS ) {
		debug("Opening streams.\n");
		result = streams_open(&stream_in, options->filename_in,
				&stream_out, options->filename_out);
	}

	if ( result == PC_SUCCESS ) {
		debug("Dispatching.\n");
		dispatch(stream_in, stream_out, options);
	}

	debug("Cleaning up.\n");
	pc_options_free(&options);
	streams_close(stream_in, stream_out);

	return(pc_check(result));
}
