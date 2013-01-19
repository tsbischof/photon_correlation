#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#include "files.h"
#include "modes.h"
#include "error.h"
#include "bin_intensity_t2.h"
#include "bin_intensity_t3.h"
#include "histogram_gn.h"
#include "options.h"

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options;

	FILE *stream_in = NULL;
	FILE *stream_out = NULL;

	program_options_t program_options = {
"This program calculates the intensities in each bin of a correlation, as \n"
"required for exact normalization of the correlation. The input is either t2\n"
"or t3 data, and the output roughly follows that of the histogram:\n"
"     t1-t0 lower, t1-t0 upper, channel 0 counts, channel 1 counts\n"
"\n"
"For the best results, pass the same arguments for time and pulse bin\n"
"definitions as for histogram.\n",
		{OPT_HELP, OPT_VERBOSE, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_BINARY_IN, OPT_BINARY_OUT,
			OPT_USE_VOID,
			OPT_MODE, OPT_CHANNELS, OPT_ORDER, 
			OPT_QUEUE_SIZE,
			OPT_TIME, OPT_PULSE,
			OPT_TIME_SCALE, OPT_PULSE_SCALE, OPT_EOF}};

	result = parse_options(argc, argv, &options, &program_options);

	if ( result == PC_SUCCESS ) {
		result = open_streams(&stream_in, options.filename_in,
				&stream_out, options.filename_out);
	}

	if ( result == PC_SUCCESS ) {
		result = bin_intensity_dispatch(stream_in, stream_out, &options);
	}

	if ( result == EOF ) {
		result = PC_SUCCESS;
	}

	free_options(&options);
	free_streams(stream_in, stream_out);

	return(pc_check(result));
}
