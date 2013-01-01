#include <stdio.h>

#include "t3.h"
#include "t2.h"
#include "options.h"
#include "files.h"
#include "error.h"

int main(int argc, char *argv[]) {
	options_t options;

	t2_t t2_photon;
	t3_t t3_photon;

	int result = 0;

	FILE *stream_in = NULL;
	FILE *stream_out = NULL;

	program_options_t program_options = {
		7,
"This program strips time information from t3 data, leaving only the pulse\n"
"number and yielding a t2-like photon.\n",
		{OPT_HELP, OPT_VERSION, OPT_VERBOSE, OPT_FILE_IN, OPT_FILE_OUT,
				OPT_BINARY_IN, OPT_BINARY_OUT}};

	result = parse_options(argc, argv, &options, &program_options);

	result += open_streams(&stream_in, options.in_filename,
				&stream_out, options.out_filename);

	if ( ! result ) {
		while ( ! next_t3(stream_in, &t3_photon, &options) ) {
			t2_photon.channel = t3_photon.channel;
			t2_photon.time = t3_photon.pulse;

			print_t2(stream_out, &t2_photon, NEWLINE, &options);
		}
	}

	free_options(&options);
	free_streams(stream_in, stream_out);

	return(parse_result(result));
}
				
