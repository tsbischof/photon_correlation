#include <stdio.h>

#include "options.h"
#include "picoquant.h"
#include "correlate.h"
#include "intensity.h"
#include "bin_intensity.h"
#include "channels.h"
#include "error.h"
#include "files.h"
#include "gn.h"

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options;

	FILE *in_stream = NULL;
	FILE *out_stream = NULL;

	program_options_t program_options = {
		27,
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
			OPT_BINARY_IN, OPT_BINARY_OUT, 
			OPT_QUEUE_SIZE, 
			OPT_START_TIME, OPT_STOP_TIME, 
			OPT_MAX_TIME_DISTANCE, OPT_MIN_TIME_DISTANCE, 
			OPT_MAX_PULSE_DISTANCE, OPT_MAX_PULSE_DISTANCE, 
			OPT_POSITIVE_ONLY, 
			OPT_TIME, OPT_PULSE, OPT_TIME_SCALE, OPT_PULSE_SCALE,
			OPT_TIME_OFFSETS, OPT_PULSE_OFFSETS,
			OPT_SUPPRESS,
			OPT_APPROXIMATE, OPT_TRUE_CORRELATION}};

	result += parse_options(argc, argv, &options, &program_options);
	result += open_streams(&in_stream, options.in_filename,
			&out_stream, options.out_filename);

	if ( ! result ) {
		gn_raw(in_stream, out_stream, &options);
	}

	free_options(&options);
	free_streams(in_stream, out_stream);

	return(parse_result(result));
}


