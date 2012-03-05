#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "files.h"
#include "modes.h"
#include "strings.h"
#include "error.h"
#include "histogram.h"
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
"      -D, --time-scale: Sets whether the time scale is \"linear\" or \n"
"                        \"log\". The default is a linear scale.\n"
"     -E, --pulse-scale: Sets whether the pulse scale is \"linear\" or \n"
"                        \"log\". The default is a linear scale.\n"
"\n"
"            This program assumes that the channels are presented in order.\n");
}

int str_to_limits(char *str, limits_t *limits) {
	int result;

	debug("Parsing limits: %s.\n", str);

	if ( str == NULL ) {
		error("Fatal error, no limits specified.\n");
		return(-1);
	}

	result = sscanf(str, "%lld,%u,%lld", &(limits->lower), &(limits->bins),
				&(limits->upper));

	if ( limits->lower >= limits->upper ) {
		error("Lower limit must be less than upper limit "
				"(%lld, %lld specified)\n", limits->lower, limits->upper);
		return(-1);
	}

	if( result != 3 ) {
		error("Limits could not be parsed: %s.\n"
				"The correct format is lower,bins,upper (no spaces).\n");
		return(-1);
	}

	return(0);
}

int scale_parse(char *str, int *scale) {
	if ( str == NULL ) {
		*scale = SCALE_LINEAR;
	} else {
		if ( !strcmp(str, "log") ) {
			*scale = SCALE_LOG;
		} else if ( !strcmp(str, "linear") ) {
			*scale = SCALE_LINEAR;
		} else {
			*scale = SCALE_UNKNOWN;
			error("Scale specified but not recognized: %s\n", str);
		}
	}
	
	return(*scale == SCALE_UNKNOWN);
}

/* g1 histogram routines. This is useful mostly for t3 mdoe, but put it here
 * in case it is useful elsewhere.
 */

g1_histograms_t *allocate_g1_histograms(limits_t *limits, int channels) {
	int result = 0;
	int i;
	int j;
	g1_histograms_t *histograms = NULL;

	histograms = (g1_histograms_t *)malloc(sizeof(g1_histograms_t));
	if ( histograms == NULL ) {
		result = -1;
	} else {
		histograms->n_histograms = channels;
		histograms->limits.lower = limits->lower;
		histograms->limits.bins = limits->bins;
		histograms->limits.upper = limits->upper; 
		histograms->bin_edges = (long long int *)malloc(sizeof(long long int)*
				limits->bins);
		histograms->histograms = (g1_histogram_t *)malloc(
				sizeof(g1_histogram_t)*histograms->n_histograms);

		if ( histograms->histograms == NULL 
				|| histograms->bin_edges == NULL ) {
			result = -1;
		} else {
			for ( i = 0; i < histograms->n_histograms; i++ ) {
				histograms->histograms[i].counts = (unsigned int *)malloc(
						sizeof(unsigned int)*histograms->limits.bins);
				if ( histograms->histograms[i].counts == NULL ) {
					result = -1;
					i = histograms->n_histograms;
				} else {
					printf("%d,%d\n", i, j);
					for ( j = 0; j < histograms->limits.bins; j++ ) {
						histograms->histograms[i].counts[j] = 0;
					}
				}
			}
		}
	}

	if ( result ) {
		error("Could not allocate histograms.\n");
		free_g1_histograms(&histograms);
	}

	return(histograms);
}

void free_g1_histograms(g1_histograms_t **histograms) {
	int i;
	if ( *histograms != NULL ) {
		free((*histograms)->bin_edges);

		if ( (*histograms)->histograms != NULL ) {
			for ( i = 0; i < (*histograms)->n_histograms; i++ ) {
				free((*histograms)->histograms[i].counts);
			}
		}

		free((*histograms)->histograms);
	}	
}

void print_g1_histograms(FILE *out_stream, g1_histograms_t *histograms) {
	int histogram_index;
	int bin_index;

	for ( histogram_index = 0; histogram_index < histograms->n_histograms; 
			histogram_index++ ) {
		for ( bin_index = 0; bin_index < histograms->limits.bins; 
				bin_index++ ) {
			fprintf(out_stream, "%d,%lld,%u\n", 
					histogram_index, 
					histograms->bin_edges[bin_index], 
					histograms->histograms[histogram_index].counts[bin_index]);
		}
	}
}

int increment_g1_histograms(g1_histograms_t *histograms,
		unsigned int channel, long long int value) {
	return(0);
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
		{"time-scale", required_argument, 0, 'd'},
		{"pulse-scale", required_argument, 0, 'e'},
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

	if ( options.order > 4 ) {
		warn("A correlation of order %d will require extensive "
				"time to compute.\n", options.order);
	} else if ( options.order < 1 ) {
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
