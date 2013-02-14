#include <stdio.h>
#include <stdlib.h>

#include "modes.h"
#include "error.h"
#include "files.h"
#include "histogram.h"
#include "options.h"

int main(int argc, char *argv[]) {
	int result = 0;

	options_t options = {};

	FILE *stream_in = NULL;
	FILE *stream_out = NULL;

	program_options_t program_options = {
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
		{OPT_HELP, OPT_VERBOSE, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_CHANNELS, OPT_ORDER,
			OPT_TIME, OPT_PULSE, OPT_TIME_SCALE, OPT_PULSE_SCALE, OPT_EOF}};

	result = parse_options(argc, argv, &options, &program_options);

	if ( result == PC_SUCCESS ) {
		result = open_streams(&stream_in, options.filename_in,
				&stream_out, options.filename_out);
	}

	if ( result == PC_SUCCESS ) {
		result = histogram_dispatch(stream_in, stream_out, &options);
	} 
			
	free_options(&options);
	free_streams(stream_in, stream_out);

	return(pc_check(result));
}
