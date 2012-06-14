#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#include "files.h"
#include "modes.h"
#include "error.h"
#include "strings.h"
#include "bin_intensity.h"
#include "bin_intensity_t2.h"
#include "bin_intensity_t3.h"
#include "t2.h"
#include "t3.h"
#include "histogram_gn.h"
#include "options.h"

#define QUEUE_SIZE 100000

void usage(void) {
	fprintf(stdout, 
"Usage: bin_intensity [-v] [-i file_in] [-o file_out] [-c channels] \n"
"                 -d time [-D time-scale]\n"
"                 -e pulse [-E pulse-scale]\n"
"                 -m mode\n"
"\n"
"       Version %d.%d\n"
"\n"
"           -v, --verbose: Print debug-level information.\n"
"           -i, --file-in: Input file. By default, this is STDIN.\n"
"          -o, --file-out: Output file. By default, this is STDOUT.\n"
"              -m, --mode: Stream type. This is either t2 or t3, and the\n"
"                          style of the output will be different for each.\n"
"                          Currently, only t2 is supported.\n"
"          -c, --channels: Number of channels in the stream. By default, this\n"
"                          is 2.\n"
"        -q, --queue-size: Defines the maximum length of the circular queue\n"
"                          held in memory for processing. By default, this \n"
"                          is %d\n"
"       -d <time>, --time: Time bin definitions, as in histogram.\n"
"        -D, --time-scale: Time scale definition, as in histogram.\n"
"     -e <pulse>, --pulse: Pulse bin definitions, as in histogram.\n"
"       -E, --pulse-scale: Pulse scale definition, as in histogram.\n"
"              -h, --help: Print this message.\n"
"\n"
"       This program assumes the input stream is time-ordered.\n",
				VERSION_MAJOR, VERSION_MINOR,
				QUEUE_SIZE);
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
		{"channels", required_argument, 0, 'c'},
		{"queue-size", required_argument, 0, 'q'},
		{"time", required_argument, 0, 'd'},
		{"time-scale", required_argument, 0, 'D'},
		{"pulse", required_argument, 0, 'e'},
		{"pulse-scale", required_argument, 0, 'E'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}};

	/* Some default values. */
	options.in_filename = NULL;
	options.out_filename = NULL;
	options.mode_string = NULL;
	options.mode = MODE_UNKNOWN;
	options.channels = 2;
	options.queue_size = QUEUE_SIZE;
	options.time_scale = 0;
	options.time_string = NULL;
	options.time_scale_string = NULL;
	options.time_scale = SCALE_LINEAR;
	options.pulse_string = NULL;
	options.pulse_scale_string = NULL;
	options.pulse_scale = SCALE_LINEAR;


	while ( (c = getopt_long(argc, argv, "hvi:o:m:c:q:d:D:e:E:", long_options,
				&option_index)) != -1 ) {
		switch (c) { 
			case 'h':
				usage();
				return(0);
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
			case 'c':
				options.channels = strtol(optarg, NULL, 10);
				break;
			case 'q':
				options.queue_size = strtol(optarg, NULL, 10);
				break;
			case 'd':
				options.time_string = strdup(optarg);
				break;
			case 'D':
				options.time_scale_string = strdup(optarg);
				break;
			case 'e':
				options.pulse_string = strdup(optarg);
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

	if ( options.mode == MODE_T2 ) {
		result += scale_parse(options.time_scale_string, &(options.time_scale));
		result += str_to_limits(options.time_string, &(options.time_limits));
	} else if ( options.mode == MODE_T3 ) {
		result += scale_parse(options.pulse_scale_string, 
					&(options.pulse_scale));
		result += str_to_limits(options.pulse_string,
					&(options.pulse_limits));
	} else {
		error("Mode not recognized: %s\n", options.mode_string);
	}

	if ( result ) {
		error("Fatal error, could not begin the calculation.\n");
	} else {
		debug("Checking the mode.\n");
		if ( options.mode == MODE_T2 ) {
			debug("Mode t2.\n");
			result = bin_intensity_t2(in_stream, out_stream, &options);
		} else if ( options.mode == MODE_T3 ) {
			debug("Mode t3.\n");
			error("Mode t3 not yet supported.\n");
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
	free(options.time_scale_string);
	free(options.pulse_string);
	free(options.pulse_scale_string);
	
	return(result);
}
