#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "modes.h"
#include "strings.h"
#include "error.h"
#include "histogram.h"
#include "histogram_gn.h"
#include "histogram_t2.h"
#include "histogram_t3.h"
#include "options.h"

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options;

	program_options_t program_options = {
		11,
		"",
		{OPT_HELP, OPT_VERBOSE,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_CHANNELS, OPT_ORDER,
			OPT_TIME, OPT_PULSE, OPT_TIME_SCALE, OPT_PULSE_SCALE}};

	result = parse_options(argc, argv, &options, &program_options);

	if ( ! result ) {
		debug("Checking the mode.\n");
		if ( options.mode == MODE_T2 ) {
			debug("Mode t2.\n");
			result = histogram_t2(options.in_stream, options.out_stream,
									 &options);
		} else if ( options.mode == MODE_T3 ) {
			debug("Mode t3.\n");
			result =  histogram_t3(options.in_stream, options.out_stream,
									&options);
		}
	}
			
	free_options(&options);
	return(result);
}
