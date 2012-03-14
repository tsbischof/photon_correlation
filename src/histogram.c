#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "files.h"
#include "modes.h"
#include "strings.h"
#include "error.h"
#include "histogram.h"
#include "histogram_gn.h"
#include "histogram_t2.h"
#include "histogram_t3.h"

void usage(void) {
	fprintf(stdout,
"Usage: histogram [-v] [-i file_in] [-o file_out] [-p print_every]\n"
"                 -d <left_time_limit, time_increment, right_time_limit> \n"
"                 -e <left_pulse_limit, pulse_increment, right_pulse_limit> \n"
"                 -c channels -g order -m mode\n"
"\n"
"         -v, --verbose: Print debug-level information.\n"
"         -i, --file-in: Input file. By default, this is stdin\n"
"        -o, --file-out: Output file. By default, this is stdout.\n"
"            -m, --mode: Stream type. This is either t2 or t3, and the\n"
"                        style of the output will be different for each.\n"
"     -d <time>, --time: The upper and lower bounds for the time axis in the\n"
"                        histogram, along with the number of bins to create.\n"
"                        the required format is lower,bins,upper (no spaces).\n"
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
"\n"
"            This program assumes that the channels are presented in order.\n");
}

int main(int argc, char *argv[]) {
	int c;
	int option_index = 0;
	FILE *in_stream = NULL;
	FILE *out_stream = NULL;
	int result = 0;

	options_t options;
	static struct option long_options[] = {
		{"verbose", no_argument, 0, 'v'},
		{"file-in", required_argument, 0, 'i'},
		{"file-out", required_argument, 0, 'o'},
		{"mode", required_argument, 0, 'm'},
		{"time", required_argument, 0, 'd'},
		{"pulse", required_argument, 0, 'e'},
		{"channels", required_argument, 0, 'c'},
		{"order", required_argument, 0, 'g'},
		{"time-scale", required_argument, 0, 'D'},
		{"pulse-scale", required_argument, 0, 'E'},
		{0, 0, 0, 0}};

	options.in_filename = NULL;
	options.out_filename = NULL;
	options.mode_string = NULL;
	options.mode = MODE_UNKNOWN;
	options.time_string = NULL;
	options.pulse_string = NULL;
	options.channels = 2;
	options.order = 2;
	options.time_scale_string = NULL;
	options.time_scale = SCALE_LINEAR;
	options.pulse_scale_string = NULL;
	options.pulse_scale = SCALE_LINEAR;

	while ( (c = getopt_long(argc, argv, "vi:o:m:d:e:c:g:D:E:", long_options,
				&option_index)) != -1 ) {
		switch (c) {
			case 'v':
				verbose = 1;
				break;
			case 'i':
				options.in_filename = strdup(optarg);
				break;
			case 'o':
				options.out_filename = strdup(optarg);
				break;
			case 'm':
				options.mode_string = strdup(optarg);
				break;
			case 'd':
				options.time_string = strdup(optarg);
				break;
			case 'e':
				options.pulse_string = strdup(optarg);
				break;
			case 'c':
				options.channels = strtol(optarg, NULL, 10);
				break;
			case 'g':
				options.order = strtol(optarg, NULL, 10);
				break;
			case 'D':
				options.time_scale_string = strdup(optarg);
				break;
			case 'E':
				options.pulse_scale_string = strdup(optarg);
				break;
			case '?':
				usage();
				return(-1);
			default:
				usage();
				return(-1);
		}
	}

	
	/* Parse the options and make sure they are reasonable. */
	result += stream_open(&in_stream, stdin, options.in_filename, "r");
	result += stream_open(&out_stream, stdout, options.out_filename, "w");

	result += mode_parse(&(options.mode), options.mode_string);

	if ( options.channels < 1 ) {
		error("Must have at least one channel (%d specified).\n", 
				options.channels);
		result += -1;
	}

	if ( options.order < 1 ) {
		error("Order of histogram must be at least 1 (%d specified).\n",
				options.order);
		result += -1;
	}

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
		

	/* Free memory. */
	debug("Cleaning up.\n");
	free(options.in_filename);
	free(options.out_filename);
	stream_close(in_stream, stdin);
	stream_close(out_stream, stdout);
	free(options.mode_string);
	free(options.time_string);
	free(options.pulse_string);
	free(options.time_scale_string);
	free(options.pulse_scale_string);
	
	return(result);
}
