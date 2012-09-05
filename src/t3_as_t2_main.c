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

	FILE *in_stream = NULL;
	FILE *out_stream = NULL;

	program_options_t program_options = {
		7,
"This program strips time information from t3 data, leaving only the pulse\n"
"number and yielding a t2-like photon.\n",
		{OPT_HELP, OPT_VERSION, OPT_VERBOSE, OPT_FILE_IN, OPT_FILE_OUT,
				OPT_BINARY_IN, OPT_BINARY_OUT}};

	result = parse_options(argc, argv, &options, &program_options);

	result += open_streams(&in_stream, options.in_filename,
				&out_stream, options.out_filename);

	if ( ! result ) {
		while ( ! next_t3(in_stream, &t3_photon, &options) ) {
			t2_photon.channel = t3_photon.channel;
			t2_photon.time = t3_photon.pulse_number;

			print_t2(out_stream, &t2_photon, NEWLINE, &options);
			fprintf(out_stream, "\n");
		}
	}

	free_options(&options);
	free_streams(in_stream, out_stream);

	return(parse_result(result));
}
				
