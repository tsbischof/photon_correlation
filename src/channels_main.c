#include <stdio.h>

#include "files.h"
#include "error.h"
#include "options.h"
#include "channels.h"

/* Tools for manipulating channels (time/pulse offset, suppression) */

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options;

	FILE *stream_in = NULL;
	FILE *stream_out = NULL;

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
	result += open_streams(&stream_in, options.in_filename,
			&stream_out, options.out_filename);

	if ( ! result ) {
		result = channels_dispatch(stream_in, stream_out, &options);
	}

	free_options(&options);
	free_streams(stream_in, stream_out);

	return(parse_result(result));
}
