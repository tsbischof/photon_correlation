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
		if ( (! options->count_all) && record.time > bin_upper_limit ) {
			print_counts(out_stream, 
					bin_upper_limit-options->bin_width, 
					bin_upper_limit,
					counts);
			bin_upper_limit += options->bin_width;
			init_counts(counts);
		} else {
			increment_counts(counts, record.channel);
			done = next_t2(in_stream, &record);
		}
	}

	print_counts(out_stream, 
			bin_upper_limit - options->bin_width, 
			record.time, 
			counts);

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
		if ( (! options->count_all) && record.pulse_number > bin_upper_limit ) {
			print_counts(out_stream, 
					bin_upper_limit-options->bin_width, 
					bin_upper_limit,
					counts);
			bin_upper_limit += options->bin_width;
			init_counts(counts);
		} else {
			increment_counts(counts, record.channel);
			done = next_t3(in_stream, &record);
		}
	}

	print_counts(out_stream, 
			bin_upper_limit - options->bin_width,
			record.pulse_number, 
			counts);

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

void print_counts(FILE *out_stream, long long int lower_time,
		long long int upper_time,  counts_t *counts) {
	int i;

	fprintf(out_stream, "%lld,%lld,", lower_time, upper_time);
	for ( i = 0; i < counts->channels; i++ ) {
		fprintf(out_stream, "%lld", counts->counts[i]);
		if ( i != counts->channels - 1 ) {
			fprintf(out_stream, ",");
		}
	}
	fprintf(out_stream, "\n");
}
		

int main(int argc, char *argv[]) {
	options_t options;
	int result = 0;

	program_options_t program_options = {
		8,
		"",
		{OPT_HELP, OPT_VERBOSE,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_CHANNELS,
			OPT_BIN_WIDTH, OPT_COUNT_ALL}};

	result = parse_options(argc, argv, &options, &program_options);

	if ( result ) {
		error("Fatal error, could not begin the calculation.\n");
	} else {
		debug("Checking the mode.\n");
		if ( options.mode == MODE_T2 ) {
			debug("Mode t2.\n");
			result = intensity_t2(options.in_stream, options.out_stream, 
								&options);
		} else if ( options.mode == MODE_T3 ) {
			debug("Mode t3.\n");
			result = intensity_t3(options.in_stream, options.out_stream, 
								&options);
		} 
	}

	free_options(&options);
	
	return(result);
}
