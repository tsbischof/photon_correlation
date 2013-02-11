#include <stdio.h>

#include "t3.h"
#include "t2.h"
#include "options.h"
#include "files.h"
#include "error.h"

int main(int argc, char *argv[]) {
	options_t options;

	t2_t t2;
	t2_next_t t2_next = t2_fscanf;
	t3_t t3;
	t3_print_t t3_print = t3_fprintf;

	int result = 0;

	FILE *stream_in = NULL;
	FILE *stream_out = NULL;

	program_options_t program_options = {
"This program uses the specified repetition time to transform t2 data into t3\n"
"data. The origin in time is assumed to be 0, and the pulse number is\n"
"produced by assuming that the given repetition time is consistent over the\n"
"entire experiment.",
		{OPT_HELP, OPT_VERSION, OPT_VERBOSE, 
				OPT_FILE_IN, OPT_FILE_OUT,
				OPT_REPETITION_TIME,
				OPT_EOF}};

	result = parse_options(argc, argv, &options, &program_options);

	if ( result == PC_SUCCESS ) {
		result = open_streams(&stream_in, options.filename_in,
					&stream_out, options.filename_out);
	} 

	if ( result == PC_SUCCESS ) {
		if ( options.repetition_time == 0 ) {
			error("Must specify non-zero repetition time.\n");
			result = PC_ERROR_ZERO_DIVISION;
		}
	}

	if ( result == PC_SUCCESS ) {
		while ( t2_next(stream_in, &t2) == PC_SUCCESS ) {
			t3.channel = t2.channel;
			t3.pulse = t2.time / options.repetition_time;
			t3.time = t2.time % options.repetition_time;

			t3_print(stream_out, &t3);
		}
	}

	if ( result == EOF ) {
		result = PC_SUCCESS;
	}

	free_options(&options);
	free_streams(stream_in, stream_out);

	return(pc_check(result));
}
