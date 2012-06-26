#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "strings.h"
#include "correlate.h"
#include "correlate_t2.h"
#include "correlate_t3.h"
#include "error.h"
#include "files.h"
#include "modes.h"
#include "options.h"

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options;

	program_options_t program_options = {
		13,
		"This program accepts TTTR photon data and outputs a stream\n"
		"of correlation events. For example, a correlation order 2 of\n"
		"t2 data will put out events of the form:\n"
		"          channel 0, channel 1, time difference\n"
		"Extension to higher orders results in an (n-1)-tupe of \n"
		"photon/time pairs, ",
		{OPT_VERBOSE, OPT_HELP,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_ORDER,
			OPT_PRINT_EVERY, OPT_POSITIVE_ONLY,
			OPT_QUEUE_SIZE,
			OPT_MAX_TIME_DISTANCE, OPT_MIN_TIME_DISTANCE,
			OPT_MAX_PULSE_DISTANCE, OPT_MIN_PULSE_DISTANCE}};

	
	result = parse_options(argc, argv, &options, &program_options);

	/* Begin the calculation. */
	if ( result ) {
		error("Fatal error, could not begin the calculation.\n");
	} else {
		debug("Checking the mode.\n");
		if ( options.mode == MODE_T2 ) {
			debug("Mode t2.\n");
			result = correlate_t2(options.in_stream, options.out_stream,
						 &options);
		} else if ( options.mode == MODE_T3 ) {
			debug("Mode t3.\n");
			result = correlate_t3(options.in_stream, options.out_stream,
						 &options);
		} 
	}

	/* Free memory. */
	free_options(&options);
	
	return(result);
}
