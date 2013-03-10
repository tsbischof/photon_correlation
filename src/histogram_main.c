#include "run.h"
#include "options.h"
#include "histogram.h"

int main(int argc, char *argv[]) {
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

	return(run(&program_options, histogram_dispatch, argc, argv));
}
