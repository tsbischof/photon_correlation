#include <stdio.h>
#include <math.h>

#include "t3.h"
#include "t2.h"
#include "options.h"
#include "files.h"
#include "error.h"
#include "conversions.h"

int main(int argc, char *argv[]) {
	options_t options = {};

	t2_t t2 = {};
	t2_next_t t2_next = t2_fscanf;
	t3_t t3 = {};
	t3_print_t t3_print = t3_fprintf;

	int result = PC_SUCCESS;

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
		if ( options.repetition_rate == 0 ) {
			error("Must specify non-zero repetition rate.\n");
			result = PC_ERROR_ZERO_DIVISION;
		} else if ( options.repetition_rate > 1e12 ) {
			error("Repetition rate must be less than %.2lg (%.2lg given)\n",
					1e12, options.repetition_rate);
			result = PC_ERROR_ZERO_DIVISION;
		}
	}

	if ( result == PC_SUCCESS ) {
		while ( t2_next(stream_in, &t2) == PC_SUCCESS ) {
			t2_to_t3(&t2, &t3, options.repetition_rate);
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
