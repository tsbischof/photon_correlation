#include <stdio.h>
#include <math.h>

#include "t3.h"
#include "t2.h"
#include "options.h"
#include "files.h"
#include "error.h"
#include "conversions.h"

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
"This program uses the specified repetition time to transform t3 data into t2\n"
"data. The origin in time is assumed to be 0, and the repetition rate is \n"
"assumed to consistent over the entire experiment.",
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
			error("Repetition rate is higher than the resolution of "
					"the data: %.2lg (limit %.2lg)\n", 
					options.repetition_rate, 1e12);
			result = PC_ERROR_ZERO_DIVISION;
		}
	}

	if ( result == PC_SUCCESS ) {
		while ( t3_next(stream_in, &t3) == PC_SUCCESS ) {
			t3_to_t2(&t3, &t2, options.repetition_rate);

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
