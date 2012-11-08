#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

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

	FILE *in_stream = NULL;
	FILE *out_stream = NULL;

	program_options_t program_options = {
		15,
"This program accepts TTTR photon data and outputs a stream of correlation \n"
"events. For example, a correlation order 2 of t2 data will put out events of\n"
"the form:\n"
"          channel 0, channel 1, time difference\n"
"Extension to higher orders results in an (n-1)-tuple of photon/time pairs,\n"
"such as:\n"
"          channel 0, channel 1, t1-t0, channel 2, t2-t0\n"
"\n"
"The input modes are:\n"
"	t2: channel, time\n"
"   t3: channel, pulse, time\n"
"\n"
"By default, all photons will be correlated with all other photons, but this\n"
"frequently becomes impractical. To avoid this, pass as arguments\n"
"--min/max-time/pulse-distance to specify the lower and upper limits of \n"
"relative time/pulse delays to calculate.\n"
"\n"
"For calculating logarithmic-scale correlations, it is helpful to pass the\n"
"--positive-only flag, which restricts the correlations to those whose \n"
"photons are presented in temporal order.\n"
"\n"
"To mimic the start-stop mode of the original correlators, pass --start-stop.\n"
"as an argument.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_ORDER,
			OPT_PRINT_EVERY, OPT_POSITIVE_ONLY, OPT_START_STOP,
			OPT_QUEUE_SIZE,
			OPT_MAX_TIME_DISTANCE, OPT_MIN_TIME_DISTANCE,
			OPT_MAX_PULSE_DISTANCE, OPT_MIN_PULSE_DISTANCE}};

	
	result = parse_options(argc, argv, &options, &program_options);

	result += open_streams(&in_stream, options.in_filename,
			&out_stream, options.out_filename);

	/* Begin the calculation. */
	if ( ! result ) {
		result = correlate_dispatch(in_stream, out_stream, &options);
	}

	/* Free memory. */
	free_options(&options);
	free_streams(in_stream, out_stream);
	
	return(parse_result(result));
}
