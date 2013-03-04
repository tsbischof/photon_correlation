#include <stdio.h>

#include "options.h"
#include "error.h"
#include "files.h"
#include "gn.h"

#define ABS(x) ( x >= 0 ? x : -x )
#define MAX(x, y) ( x < y ? y : x )

int main(int argc, char *argv[]) {
	int result = PC_SUCCESS;

	options_t options;

	FILE *stream_in = NULL;
	FILE *stream_out = NULL;

	program_options_t program_options = {
"This program performs the calculations needed to calculate the correlation\n"
"of a stream of photons, including those needed to normalize the result.\n"
"It does not perform the normalization, but instead produces the results\n"
"necessary to do so.\n"
"\n"
"For a given input file, some number of files will be output:\n"
" 1. intensity data\n"
"    a. *.count-all: The total number of counts found on all channels, useful\n"
"       for approximate normalization. By default, this is the output.\n"
"    b. *.bin_intensity: The exact values needed to normalize each histogram\n"
"       bin. This is more expensive to calculate than .count-all, so only\n"
"       use this if the exact result is desired.\n"
" 2. Correlation data\n"
"    *.gn: The n is the appropriate value for the given correlation. This\n"
"    file contains the histogrammed correlation events, and is the non-\n"
"    normalized correlation.\n"
"\n"
"These files can be assembled together to form:\n"
" a. *.gn.norm: Using *.count-all, calculate the approximate correlation\n"
"    function.\n"
" b. *.gn.norm-exact: Using *.bin_intensity, calculate the exact correlation\n"
"    function.\n"
"\n"
"The assembly of the normalized functions is handled externally, by a Python\n"
"script.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION, 
			OPT_FILE_IN, OPT_FILE_OUT, 
			OPT_MODE, OPT_CHANNELS, OPT_ORDER, 
			OPT_QUEUE_SIZE, 
			OPT_TIME, OPT_PULSE, OPT_TIME_SCALE, OPT_PULSE_SCALE,
			OPT_EXACT_NORMALIZATION, OPT_EOF}};

	result = parse_options(argc, argv, &options, &program_options);

	if ( result == PC_SUCCESS ) {
		debug("Starting gn.\n");
		options.max_time_distance = MAX(ABS(options.time_limits.lower),
				ABS(options.time_limits.upper));
		options.max_pulse_distance = MAX(ABS(options.pulse_limits.lower),
				ABS(options.pulse_limits.upper));
		gn(options.filename_in, &options);
	}

	free_options(&options);
	free_streams(stream_in, stream_out);

	return(pc_check(result));
}


