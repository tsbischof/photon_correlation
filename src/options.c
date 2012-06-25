#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>

#include "options.h"
#include "error.h"
#include "files.h"
#include "modes.h"

int parse_options(int argc, char *argv[], options_t *options, 
		char *options_string) {
	int c;
	int option_index = 0;
	int result;

	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"verbose", no_argument, 0, 'v'},
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

/* Intensity */ 
		{"bin-width", required_argument, 0, 'w'},
		{"count-all", required_argument, 0, 'A'},

/* Histogram */ 
		{"time", required_argument, 0, 'x'},
		{"pulse", required_argument, 0, 'y'},
		{"time-scale", required_argument, 0, 'X'},
		{"pulse-scale", required_argument, 0, 'Y'},

		{0, 0, 0, 0}}

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

	options->bin_width = 0;
	options->print_last = 0;
	options->count_all = 0;

	options->time_string = NULL;
	options->pulse_string = NULL;

	options->time_scale_string = NULL;
	options->time_scale = SCALE_LINEAR;
	options->pulse_scale_string = NULL;
	options->pulse_scale = SCALE_LINEAR;
	
	while ( (c = getopt_long(argc, argv, options_string,
						long_options, &option_index)) != 1 ) {
		switch (c) {
			case 'h':
				break;
			case 'v':
				verbose = 1;
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
				options->number = strtoll(optarg);
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
				options->time_string = strdup(optarg, NULL, 10);
				break;
			case 'y':
				options->pulse_string = strdup(optarg, NULL, 10);
				break;
			case 'X':
				options->time_scale_string = strdup(optarg, NULL, 10);
				break;
			case 'Y':
				options->pulse_scale_string = strdup(optarg, NULL, 10);
				break;
			case '?':
			default:
				usage(argc, argv, options_string);
				return(-1);
		}
	}

	if ( options->binary_in || options->binary_out ) {
		warn("Binary file mode not yet supported.\n");
	}

	result += stream_open(&(options->in_stream), 
					stdin, options->in_filename, "r");
	result += stream_open(&(options->out_stream),
					stdout, options->out_filename, "w");
		
	if ( is_option('c', options_string) && option->channels < 1 ) {
		error("Must have at least 1 channel (%d specified).\n", 
				options->channels);
		result += -1;
	}

	if ( is_option('g', options_string) && options->order < 1 ) {
		error("Order of correlation/histogram must be at least 1 (%d "
				"specified).", options->order);
		result += -1;
	}

	if ( is_option('m', options_string) ) {
		result += mode_parse(&(options->mode), options->mode_string);
	}

//	dDeExyXY

		
}

void usage(int argc, char *argv[], char *options_string) {
	/* Loop through the possible options. */
	fprintf(stderr, "Usage: %s [options]\n", argv[0]);
}

int is_option(char option, char *options_string) {
	int i;

	for ( i = 0; i < strlen(options_string); i++ ) {
		if ( option == options_string[i] ) {
			return(1);
		}
	}

	return(0);
}

void free_options(options_t *options) {
	free(options->in_filename)
	free(options->out_filename);
	free(options->mode_string);
	free(options->file_type_string);
	free(options->time_string);
	free(options->pulse_string);
	free(options->time_scale_string);
	free(options->pulse_scale_string);
	
	stream_close(options->in_stream, stdin);
	stream_close(options->out_stream, stdout);
}
