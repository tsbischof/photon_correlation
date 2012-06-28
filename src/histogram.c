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

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options;

	program_options_t program_options = {
		11,
"Histogram collects a set of photon correlation events and counts them into\n"
"bins defined by their relative time delays. The histograms are also\n"
"separated by the identities of each channel in the correlation, such that\n"
"all cross-correlations can be recovered. The output is similar to the \n"
"correlation events, except that the delay limits are now given:\n"
"   (t2, order 2)\n"
"   channel 0, channel 1, t1-t0 lower, t1-t0 upper, counts\n"
"Extension to higher orders creates more channel-delay pairs, and for t3 mode\n"
"the pair becomes a channel-pulse-time tuple.\n"
"\n"
"An order 1 correlation of t3 data is possible, since this is already \n"
"implicitly a correlation of a sync source and a photon stream. This is\n"
"useful for calculating a photoluminescence lifetime, for example.\n",
		{OPT_HELP, OPT_VERBOSE,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_CHANNELS, OPT_ORDER,
			OPT_TIME, OPT_PULSE, OPT_TIME_SCALE, OPT_PULSE_SCALE}};

	result = parse_options(argc, argv, &options, &program_options);

	if ( ! result ) {
		debug("Checking the mode.\n");
		if ( options.mode == MODE_T2 ) {
			debug("Mode t2.\n");
			result = histogram_t2(options.in_stream, options.out_stream,
									 &options);
		} else if ( options.mode == MODE_T3 ) {
			debug("Mode t3.\n");
			result =  histogram_t3(options.in_stream, options.out_stream,
									&options);
		}
	}
			
	free_options(&options);

	return(parse_result(result));
}
