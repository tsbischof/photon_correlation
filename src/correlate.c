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

#define MODE_T2 2
#define MODE_T3 3

void usage(void) {
	fprintf(stdout, 
"Usage: correlate [-v] [-i file_in] [-o file_out] [-a] [-b] [-n number]\n"
"                 [-p print_every] [-q queue_size] [-d max_time_distance] \n"
"                 [-e max_pulse_distance] [-r] -g order -c channels -m mode\n"
"\n"
"       Version %d.%d\n"
"\n"
"           -v, --verbose: Print debug-level information.\n"
"           -i, --file-in: Input file. By default, this is STDIN.\n"
"          -o, --file-out: Output file. By default, this is STDOUT.\n"
"       -p, --print-every: Print the result for multiples of this number of\n"
"                          entries. Default is to print nothing.\n"
"              -m, --mode: Stream type. This is either t2 or t3, and the\n"
"                          style of the output will be different for each.\n"
"        -q, --queue-size: Defines the maximum length of the circular queue\n"
"                          held in memory for processing. By default, this\n"
"                          is %d\n"
" -d, --max-time-distance: Defines the maximum difference in time that two\n"
"                          entries can have and still be considered for \n"
"                          correlation (t2 and t3).\n"
"-e, --max-pulse-distance: Defines the maximum difference in pulse number \n"
"                          that two entries can have and still be considered\n"
"                          for correlation (t3 only).\n"
"             -g, --order: Order (g(n)(t1...tn-1)) of the correlation to\n"
"                          perform. By default this is 2, the standard\n"
"                          cross-correlation of two channels.\n"
"     -P, --positive-only: Output only the correlations with positive time\n"
"                          offsets. This is most useful for logarithmic\n"
"                          binning, where negative values could cause errors.\n"
"                          By default, all photon permutations are output.\n"
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
		{"print-every", required_argument, 0, 'p'},
		{"mode", required_argument, 0, 'm'},
		{"queue-size", required_argument, 0, 'q'},
		{"max-time-distance", required_argument, 0, 'd'},
		{"max-pulse-distance", required_argument, 0, 'e'},
		{"order", required_argument, 0, 'g'},
		{"positive-only", no_argument, 0, 'P'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}};

	/* Some default values. */
	options.in_filename = NULL;
	options.out_filename = NULL;
	options.mode_string = NULL;
	options.mode = -1;
	options.print_every = 0;
	options.queue_size = QUEUE_SIZE;
	options.max_time_distance = 0;
	options.max_pulse_distance = 0;
	options.order = 2;
	options.positive_only = 0;

	while ( (c = getopt_long(argc, argv, "hvi:o:p:m:q:d:g:r", long_options,
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
			case 'p':
				options.print_every = strtol(optarg, NULL, 10);
				break;
			case 'm':
				options.mode_string = strdup(optarg);
				break;
			case 'q':
				options.queue_size = strtoll(optarg, NULL, 10);
				break;
			case 'd':
				options.max_time_distance = strtoll(optarg, NULL, 10);
				break;
			case 'e':
				options.max_pulse_distance = strtoll(optarg, NULL, 10);
				break;
			case 'g':
				options.order = strtol(optarg, NULL, 10);
				break;
			case 'P':
				options.positive_only = 1;
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

	if ( options.mode == MODE_T2 ) {
		if ( options.max_time_distance == 0 ) {
			warn("Correlating all entries. This may take a long tine.\n");
		} else if ( options.max_time_distance < 0 ) { 
			error("Time distance cannot be less than zero (%d specified).\n", 
					options.max_time_distance);
			result += -1;
		}
	} else if ( options.mode == MODE_T3 ) {
		if ( options.max_pulse_distance == 0 ) {
			warn("Correlating all entries. This may take a long time.\n");
		} else if ( options.max_pulse_distance < 0 ) {
			error("Pulse distance cannot be less than zero (%d specified).\n",
					options.max_pulse_distance);
			result += -1;
		}
	}

	if ( options.order > 4 ) {
		warn("A correlation of order %d will require extensive "
				"time to compute.\n", options.order);
	}

	/* Begin the calculation. */
	if ( result ) {
		error("Fatal error, could not begin the calculation.\n");
	} else {
		debug("Checking the mode.\n");
		if ( options.mode == MODE_T2 ) {
			debug("Mode t2.\n");
			result = correlate_t2(in_stream, out_stream, &options);
		} else if ( options.mode == MODE_T3 ) {
			debug("Mode t3.\n");
			result = correlate_t3(in_stream, out_stream, &options);
		} 
	}

	/* Free memory. */
	debug("Cleaning up.\n");
	free(options.in_filename);
	free(options.out_filename);
	stream_close(in_stream, stdin);
	stream_close(out_stream, stdout);
	free(options.mode_string);
	
	return(result);
}
