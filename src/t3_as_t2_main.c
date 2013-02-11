#include <stdio.h>

#include "t3.h"
#include "t2.h"
#include "options.h"
#include "files.h"
#include "error.h"

int main(int argc, char *argv[]) {
	options_t options;

	t2_t t2;
	t2_print_t t2_print = t2_fprintf;
	t3_t t3;
	t3_next_t t3_next = t3_fscanf;

	int result = 0;

	FILE *stream_in = NULL;
	FILE *stream_out = NULL;

	program_options_t program_options = {
"This program strips time information from t3 data, leaving only the pulse\n"
"number and yielding a t2-like photon.\n",
		{OPT_HELP, OPT_VERSION, OPT_VERBOSE, 
				OPT_FILE_IN, OPT_FILE_OUT,
				OPT_EOF}};

	result = parse_options(argc, argv, &options, &program_options);

	if ( result == PC_SUCCESS ) {
		result += open_streams(&stream_in, options.filename_in,
					&stream_out, options.filename_out);
	} 

	if ( result == PC_SUCCESS ) {
		while ( t3_next(stream_in, &t3) == PC_SUCCESS ) {
			t2.channel = t3.channel;
			t2.time = t3.pulse;

			t2_print(stream_out, &t2);
		}
	}

	if ( result == EOF ) {
		result = PC_SUCCESS;
	}

	free_options(&options);
	free_streams(stream_in, stream_out);

	return(pc_check(result));
}
				
