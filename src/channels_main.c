#include <stdio.h>

#include "files.h"
#include "error.h"
#include "options.h"
#include "channels.h"

/* Tools for manipulating channels (time/pulse offset, suppression) */

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options;

	FILE *in_stream = NULL;
	FILE *out_stream = NULL;

	program_options_t program_options = {
		11,
"This program performs some basic manipulations of tttr data. This includes\n"
"suppression of channel data (removal of all such photons from a stream)\n"
"and addition of a constant offset to particular channels.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE,
			OPT_BINARY_IN, OPT_BINARY_OUT,
			OPT_TIME_OFFSETS, OPT_PULSE_OFFSETS,
			OPT_SUPPRESS}};

	result += parse_options(argc, argv, &options, &program_options);
	result += open_streams(&in_stream, options.in_filename,
			&out_stream, options.out_filename);

	if ( ! result ) {
		result = channels_dispatch(in_stream, out_stream, &options);
	}

	free_options(&options);
	free_streams(in_stream, out_stream);

	return(parse_result(result));
}
