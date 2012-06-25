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

void usage(void) {
	fprintf(stdout,
"Usage: histogram [-v] [-i file_in] [-o file_out] [-p print_every]\n"
"                 -d <left_time_limit, time_increment, right_time_limit> \n"
"                 -e <left_pulse_limit, pulse_increment, right_pulse_limit> \n"
"                 -c channels -g order -m mode\n"
"\n"
"       Version %d.%d\n"
"\n"
"         -v, --verbose: Print debug-level information.\n"
"         -i, --file-in: Input file. By default, this is stdin\n"
"        -o, --file-out: Output file. By default, this is stdout.\n"
"            -m, --mode: Stream type. This is either t2 or t3, and the\n"
"                        style of the output will be different for each.\n"
"     -d <time>, --time: The upper and lower bounds for the time axis in the\n"
"                        histogram, along with the number of bins to create.\n"
"                        The required format is lower,bins,upper (no spaces).\n"
"   -e <pulse>, --pulse: Same as time, but for pulses. This is only relevant\n"
"                        in t3 mode.\n"
"        -c, --channels: The number of channels in the incoming stream. By\n"
"                        default, this is 2 (Picoharp or TimeHarp).\n"
"           -g, --order: The order of the correlation performed. By default,\n"
"                        this is 2 (the standard cross-correlation.\n"
"      -D, --time-scale: Sets whether the time scale is \"linear\", \n"
"                        \"log\". or \"log-zero\" (includes zero-time peak in\n"
"                        the first bin. The default is a linear scale.\n"
"     -E, --pulse-scale: Sets whether the pulse scale is \"linear\", \n"
"                        \"log\", or \"log-zero\". The default is a linear\n"
"                        scale.\n"
"            -h, --help: Print this message.\n"
"\n"
"            This program assumes that the channels are presented in order.\n",
				VERSION_MAJOR, VERSION_MINOR);
}

int main(int argc, char *argv[]) {
	int c;
	int option_index = 0;
	FILE *in_stream = NULL;
	FILE *out_stream = NULL;
	int result = 0;

	options_t options;

	/* Parse the options and make sure they are reasonable. */
	if ( parse_options(argc, argv, &options, "hviomcgxyXY") ) {
		result += mode_parse(&(options.mode), options.mode_string);

		if ( options.mode == MODE_T2 ) {
			result += scale_parse(options.time_scale_string, &(options.time_scale));
			result += str_to_limits(options.time_string, &(options.time_limits));
		} else if ( options.mode == MODE_T3 ) {
			/* For a first-order t3 correlation, we just care about the time limits,
		 	* because the pulse number is thrown out.
		 	*/
			result += scale_parse(options.time_scale_string, 
						&(options.time_scale));
			result += str_to_limits(options.time_string, 
						&(options.time_limits));

			if ( options.order != 1 ) {
				result += scale_parse(options.pulse_scale_string, 
						&(options.pulse_scale));
				result += str_to_limits(options.pulse_string, 
						&(options.pulse_limits));
			}
		} else {
			error("Mode not recognized: %s.\n", options.mode_string);
			result += -1;
		}

		/* Begin the calculation. */
		if ( result ) {
			error("Fatal error, could not begin the calculation.\n");
		} else {
			debug("Checking the mode.\n");
			if ( options.mode == MODE_T2 ) {
				debug("Mode t2.\n");
				result = histogram_t2(in_stream, out_stream, &options);
			} else if ( options.mode == MODE_T3 ) {
				debug("Mode t3.\n");
				result = histogram_t3(in_stream, out_stream, &options);
			} 
		}
	}
		
	free_options(options);
	return(result);
}
