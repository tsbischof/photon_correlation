#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>

#include "options.h"
#include "strings.h"
#include "error.h"
#include "files.h"
#include "limits.h"
#include "modes.h"

option_t all_options[] = {
	{'h', "h", "help", 
			"Prints this usage message."},
	{'V', "V", "verbose", 
			"Print debug-level information."},
	{'v', "v", "version",
			"Print version information."},
	{'p', "p:", "print-every", 
			"Print a status message every n entries.\n"
			"By default, nothing is printed."},
	{'i', "i:", "file-in", 
			"Input filename. By default, this is stdin."},
	{'o', "o:", "file-out", 
			"Output filename. By default, this is stdout."},
	{'m', "m:", "mode", 
			"TTTR mode (t2 or t3) represented by the data."},
	{'c', "c:", "channels",
			"The number of channels in the signal."},
	{'g', "g:", "order",
			"The order of the correlation or histogram."},
	{'a', "a", "binary-in",
			"Specifies that the input file is in binary format,\n"
			"rather than text."},
	{'b', "b", "binary-out",
			"Specifies that the output file is in binary format,\n"
			"rather than text."},
	{'z', "z", "resolution-only",
			"Rather than processing any data, print the \n"
			"resolution of the measurement. For TTTR modes, \n"
			"this is a single float, but for interactive data\n"
			"the resolution of each curve is given."},
	{'r', "r", "header-only",
			"Rather than processing any data, print the header\n"
			"of the file in an ini-like format. This is useful\n"
			"for debugging and verifying file settings."},
	{'t', "t", "to-t2", 
			"For t3 data, use the sync rate to determine the\n"
			"time represented by the sync count and output the\n"
			"data in t2 mode. Note that this will only be\n"
			"reasonable if the sync source is perfectly regular."},
	{'n', "n:", "number", 
			"The number of entries to process. By default, \n"
			"all entries are processed."},
	{'q', "q:", "queue-size", 
			"The size of the queue for processingi, in number of\n"
			"photons. By default, this is 100000, and if it is\n"
			"too small an appropriate warning message will be\n"
			"displayed."},
	{'d', "d:", "max-time-distance", 
			"The maximum time difference between two photons\n"
			"to be considered for the calculation."},
	{'D', "D:", "min-time-distance", 
			"The minimum time difference between two photons\n"
			"to be considered for the calculation."},
	{'e', "e:", "max-pulse-distance", 
			"The maximum pulse difference between two photons\n"
			"to be considered for the calculation."},
	{'E', "E:", "min-pulse-distance", 
			"The maximum pulse difference between two photons\n"
			"to be considered for the calculation."},
	{'P', "P", "positive-only",
			"Process only the positive-time events, that is\n"
			"only photons in their natural time order. This\n"
			"is primarily useful for calculating correlations\n"
			"on a logarithmic scale."},
	{'F', "F", "first-only",
			"Process each photon at most one time. This mimics\n"
			"the behavior of the start-stop mode of the\n"
			"physical correlator, which throws out a photon\n"
			"once it has been used."},
	{'w', "w:", "bin-width",
			"The width of the time bin for processing \n"
			"photons. For t2 mode, this is a number of\n"
			"picoseconds, and for t3 mode this is a \n"
			"number of pulses."},
	{'A', "A", "count-all", 
			"Rather than counting photons in a given time bin,\n"
			"count all photons in the stream."},
	{'x', "x:", "time", 
			"The time limits for an axis, following the format:\n"
			"        lower, number of bins, upper \n"
			"with no spaces, and the extrema in picoseconds."},
	{'y', "y:", "pulse",
			"The pulse limits for an axis, following the format:\n"
			"        lower, number of bins, upper \n"
			"with no spaces, and the extrema in picoseconds."},
	{'X', "X:", "time-scale",
			"The scale of the time axis, one of:\n"
			"   linear: linear interpolation between limits\n"
			"      log: logarithmic interpolation (linear\n"
			"           on a log axis)\n"
			" log-zero: same as log, except that the lowest\n"
			"           bin is extended to include 0"},
	{'Y', "Y:", "pulse-scale",
			"The scale of the pulse axis, one of:\n"
			"   linear: linear interpolation between limits\n"
			"      log: logarithmic interpolation (linear\n"
			"           on a log axis)\n"
			" log-zero: same as log, except that the lowest\n"
			"           bin is extended to include 0"}
	};

void default_options(options_t *options) {
	options->in_filename = NULL;
	options->out_filename = NULL;

	options->mode_string = NULL;
	options->mode = MODE_UNKNOWN;

	options->channels = 2;

	options->order = 2;
	
	options->print_every = 0;

	options->binary_in = 0;
	options->binary_out = 0;

	options->number = LLONG_MAX;
	options->print_header = 0;
	options->print_resolution = 0;
	options->to_t2 = 0;

	options->queue_size = QUEUE_SIZE;
	options->max_time_distance = 0;
	options->min_time_distance = 0;
	options->max_pulse_distance = 0;
	options->min_pulse_distance = 0;
	options->positive_only = 0;
	options->first_only = 0;

	options->bin_width = 0;
	options->count_all = 0;

	options->time_string = NULL;
	options->pulse_string = NULL;

	options->time_scale_string = NULL;
	options->time_scale = SCALE_LINEAR;
	options->pulse_scale_string = NULL;
	options->pulse_scale = SCALE_LINEAR;
}

int validate_options(program_options_t *program_options, options_t *options) {
	int result = 0;

	if ( options->binary_in || options->binary_out ) {
		warn("Binary file mode not yet supported.\n");
	}

	if ( is_option(OPT_CHANNELS, program_options) && options->channels < 1 ) {
		error("Must have at least 1 channel (%d specified).\n", 
				options->channels);
		result += -1;
	}

	if ( is_option(OPT_ORDER, program_options) && options->order < 1 ) {
		error("Order of correlation/histogram must be at least 1 (%d "
				"specified).", options->order);
		result += -1;
	}

	if ( is_option(OPT_MODE, program_options) ) {
		result += mode_parse(&(options->mode), options->mode_string);
	}

	if ( is_option(OPT_TIME_SCALE, program_options) ) {
		result += scale_parse(options->time_scale_string,
							&(options->time_scale));
	}

	if ( is_option(OPT_PULSE_SCALE, program_options) ) {
		result += scale_parse(options->pulse_scale_string,
							&(options->pulse_scale));
	}

	if ( is_option(OPT_TIME, program_options) ) {
		result += str_to_limits(options->time_string,
								&(options->time_limits));
	}

	if ( is_option(OPT_PULSE, program_options) 
		&& ! result
		&& options->mode == MODE_T3 
		&& options->order > 1 ) {
		result += str_to_limits(options->pulse_string,
								&(options->pulse_limits));
	}
		
	if ( is_option(OPT_BIN_WIDTH, program_options)
		&& ! result
		&& !(options->bin_width || options->count_all) ) {
		error("Bin width must be at least 1 (%lld specified.\n", 
				options->bin_width);
		result += -1;
	}
		
	return(result);
}

int parse_options(int argc, char *argv[], options_t *options, 
		program_options_t *program_options) {
	int c;
	int option_index = 0;
	int result = 0;
	char *options_string;

	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"verbose", no_argument, 0, 'V'},
		{"version", no_argument, 0, 'v'},
		{"print-every", required_argument, 0, 'p'},

		{"file-in", required_argument, 0, 'i'},
		{"file-out", required_argument, 0, 'o'},

		{"mode", required_argument, 0, 'm'},
		{"channels", required_argument, 0, 'c'},
		{"order", required_argument, 0, 'g'},

		{"binary-in", no_argument, 0, 'a'},
		{"binary-out", no_argument, 0, 'b'},

/* Picoquant */
		{"resolution-only", no_argument, 0, 'z'},
		{"header-only", no_argument, 0, 'r'},
		{"to-t2", no_argument, 0, 't'},
		{"number", required_argument, 0, 'n'},

/* Correlate */
		{"queue-size", required_argument, 0, 'q'},
		{"max-time-distance", required_argument, 0, 'd'},
		{"min-time-distance", required_argument, 0, 'D'},
		{"max-pulse-distance", required_argument, 0, 'e'},
		{"min-pulse-distance", required_argument, 0, 'E'},
		{"positive-only", no_argument, 0, 'P'},
		{"first-only", no_argument, 0, 'F'},

/* Intensity */ 
		{"bin-width", required_argument, 0, 'w'},
		{"count-all", no_argument, 0, 'A'},

/* Histogram */ 
		{"time", required_argument, 0, 'x'},
		{"pulse", required_argument, 0, 'y'},
		{"time-scale", required_argument, 0, 'X'},
		{"pulse-scale", required_argument, 0, 'Y'},

		{0, 0, 0, 0}};

	options_string = get_options_string(program_options);
	default_options(options);

	while ( (c = getopt_long(argc, argv, options_string,
						long_options, &option_index)) != -1 ) {
		switch (c) {
			case 'h':
				usage(argc, argv, program_options);
				result = USAGE;
				break;
			case 'V':
				verbose = 1;
				break;
			case 'v':
				version(argc, argv);
				result = -1;
				break;
			case 'p':
				options->print_every = strtol(optarg, NULL, 10);
				break;
			case 'i':
				options->in_filename = strdup(optarg);
				break;
			case 'o':
				options->out_filename = strdup(optarg);
				break;
			case 'm':
				options->mode_string = strdup(optarg);
				break;
			case 'c':
				options->channels = strtol(optarg, NULL, 10);
				break;
			case 'g':
				options->order = strtol(optarg, NULL, 10);
				break;
			case 'a':
				options->binary_in = 1;
				break;
			case 'b':
				options->binary_out = 1;
				break;
			case 'z':
				options->print_resolution = 1;
				break;
			case 'r':
				options->print_header = 1;
				break;
			case 't':
				options->to_t2 = 1;
				break;
			case 'n':
				options->number = strtoll(optarg, NULL, 10);
				break;
			case 'q':
				options->queue_size = strtoll(optarg, NULL, 10);
				break;
			case 'd':
				options->max_time_distance = strtoll(optarg, NULL, 10);
				break;
			case 'D':
				options->min_time_distance = strtoll(optarg, NULL, 10);
				break;
			case 'e':
				options->max_pulse_distance = strtoll(optarg, NULL, 10);
				break;
			case 'E':
				options->min_time_distance = strtoll(optarg, NULL, 10);
				break;
			case 'P':
				options->positive_only = 1;
				break;
			case 'w':
				options->bin_width = strtoll(optarg, NULL, 10);
				break;
			case 'A':
				options->count_all = 1;
				break;
			case 'x':
				options->time_string = strdup(optarg);
				break;
			case 'y':
				options->pulse_string = strdup(optarg);
				break;
			case 'X':
				options->time_scale_string = strdup(optarg);
				break;
			case 'Y':
				options->pulse_scale_string = strdup(optarg);
				break;
			case 'F':
				options->first_only = 1;
				break;
			case '?':
			default:
				usage(argc, argv, program_options);
				result = -1;
		}
	}

	if ( ! result ) {
		result = validate_options(program_options, options);
	}

	free(options_string);
		
	return(result);
}

void usage(int argc, char *argv[], program_options_t *program_options) {
	int i,j;
	option_t *option;

	fprintf(stderr, "Usage: %s [options]\n\n", argv[0]);
	fprintf(stderr, "Version %d.%d\n\n", VERSION_MAJOR, VERSION_MINOR);

	for ( i = 0; i < program_options->n_options; i++ ) {
		option = &all_options[program_options->options[i]];

		fprintf(stderr, "%*s-%c, --%s: ", 
				20-(int)strlen(option->long_name),
				" ",
				option->short_char,
				option->long_name);

		for ( j = 0; j < strlen(option->description); j++ ) {
			if ( option->description[j] == '\n' ) {
				fprintf(stderr, "\n%*s", 28, " ");
			} else {
				fprintf(stderr, "%c", option->description[j]);
			}
		} 

		fprintf(stderr, "\n");
	}

	fprintf(stderr, "\n%s\n", program_options->message);
}

void version(int argc, char *argv[]) {
	fprintf(stderr, "%s v%d.%d\n", argv[0], VERSION_MAJOR, VERSION_MINOR);
}

int is_option(int option, program_options_t *program_options) {
	int i;

	for ( i = 0; i < program_options->n_options; i++ ) {
		if ( option == program_options->options[i] ) {
			return(1);
		}
	}

	return(0);
}

void free_options(options_t *options) {
	free(options->in_filename);
	free(options->out_filename);
	free(options->mode_string);
	free(options->time_string);
	free(options->pulse_string);
	free(options->time_scale_string);
	free(options->pulse_scale_string);
}

char *get_options_string(program_options_t *program_options) {
	char buffer[1000];
	option_t *option;
	int i;
	int j;
	int position = 0;

	for ( i = 0; i < program_options->n_options; i++ ) {
		option = &all_options[program_options->options[i]];

		for ( j = 0; j < strlen(option->long_char); j++ ) {
			buffer[position++] = option->long_char[j];
		}
	}
	buffer[position] = '\0';
	
	return(strdup(buffer));
}
