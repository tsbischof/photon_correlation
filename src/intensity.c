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
#include "intensity.h"
#include "t2.h"
#include "t3.h"

#define MODE_T2 2
#define MODE_T3 3

void usage(void) {
	fprintf(stdout, 
"Usage: intensity [-v] [-i file_in] [-o file_out] [-c channels]\n"
"                 -w bin_width -m mode\n"
"\n"
"           -v, --verbose: Print debug-level information.\n"
"           -i, --file-in: Input file. By default, this is STDIN.\n"
"          -o, --file-out: Output file. By default, this is STDOUT.\n"
"         -w, --bin-width: Width of the bin, in time or pulses depending\n"
"                          on the mode.\n"
"              -m, --mode: Stream type. This is either t2 or t3, and the\n"
"                          style of the output will be different for each.\n"
"          -c, --channels: Number of channels in the stream. By default, this\n"
"                          is 2.\n"
"              -h, --help: Print this message.\n"
"\n"
"       This program assumes the input stream is time-ordered.\n");
}

int intensity_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	t2_t record;
	counts_t *counts;
	long long int bin_upper_limit;
	int result = 0;
	int done = 0;

	counts = allocate_counts(options->channels);

	if ( counts == NULL ) {
		result = -1;
	}

	bin_upper_limit = options->bin_width;
	init_counts(counts);

	done = next_t2(in_stream, &record);

	while ( ! result && ! done ) {
		if ( record.time > bin_upper_limit ) {
			print_counts(out_stream, 
					bin_upper_limit-options->bin_width, counts);
			bin_upper_limit += options->bin_width;
			init_counts(counts);
		} else {
			increment_counts(counts, record.channel);
			done = next_t2(in_stream, &record);
		}
	}

	free_counts(&counts);
	return(0);
}

int intensity_t3(FILE *in_stream, FILE *out_stream, options_t *options) {
	t3_t record;
	counts_t *counts;
	long long int bin_upper_limit;
	int result = 0;
	int done = 0;

	counts = allocate_counts(options->channels);

	if ( counts == NULL ) {
		result = -1;
	}

	bin_upper_limit = options->bin_width;
	init_counts(counts);

	done = next_t3(in_stream, &record);

	while ( ! result && ! done ) {
		if ( record.pulse_number > bin_upper_limit ) {
			print_counts(out_stream, 
					bin_upper_limit-options->bin_width, counts);
			bin_upper_limit += options->bin_width;
			init_counts(counts);
		} else {
			increment_counts(counts, record.channel);
			done = next_t3(in_stream, &record);
		}
	}

	free_counts(&counts);
	return(0);
}

counts_t *allocate_counts(int channels) {
	counts_t *counts;
	int result = 0;
	
	counts = (counts_t *)malloc(sizeof(counts_t));
	if ( counts == NULL ) {
		result = -1;
	} else {
		counts->channels = channels;
		counts->counts = (long long int *)malloc(
				sizeof(long long int)*channels);
		if ( counts->counts == NULL ) {
			result = -1;
		}
	}

	if ( result ) {
		free_counts(&counts);
		counts = NULL;
	}

	return(counts);
}

void init_counts(counts_t *counts) {
	int i;

	for ( i = 0; i < counts->channels; i++ ) {
		counts->counts[i] = 0;
	}
}

void free_counts(counts_t **counts) {
	if ( *counts != NULL ) {
		free((*counts)->counts);
		free(*counts);
	}
}

int increment_counts(counts_t *counts, int channel) {
	if ( channel >= 0 && channel < counts->channels ) {
		counts->counts[channel] += 1;
		return(0);
	} else {
		error("Invalid channel %d requested for increment.\n", channel);
		return(-1);
	}
}

void print_counts(FILE *out_stream, long long int time, counts_t *counts) {
	int i;

	fprintf(out_stream, "%lld,", time);
	for ( i = 0; i < counts->channels; i++ ) {
		fprintf(out_stream, "%lld", counts->counts[i]);
		if ( i != counts->channels - 1 ) {
			fprintf(out_stream, ",");
		}
	}
	fprintf(out_stream, "\n");
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
		{"bin-width", required_argument, 0, 'w'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}};

	/* Some default values. */
	options.in_filename = NULL;
	options.out_filename = NULL;
	options.mode_string = NULL;
	options.mode = -1;
	options.channels = 2;
	options.bin_width = 1;

	while ( (c = getopt_long(argc, argv, "hvi:o:m:c:w:", long_options,
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
			case 'w':
				options.bin_width = strtoll(optarg, NULL, 10);
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

	if ( result ) {
		error("Fatal error, could not begin the calculation.\n");
	} else {
		debug("Checking the mode.\n");
		if ( options.mode == MODE_T2 ) {
			debug("Mode t2.\n");
			result = intensity_t2(in_stream, out_stream, &options);
		} else if ( options.mode == MODE_T3 ) {
			debug("Mode t3.\n");
			result = intensity_t3(in_stream, out_stream, &options);
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
