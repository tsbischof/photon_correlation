#include <stdio.h>
#include <stdlib.h>

#include "files.h"
#include "error.h"
#include "intensity.h"

int main(int argc, char *argv[]) {
	options_t options;
	int result = 0;

	FILE *in_stream = NULL;
	FILE *out_stream = NULL;

	program_options_t program_options = {
		8,
"Given a stream of TTTR data formatted like the output of picoquant, this\n"
"program calculates the number of photons arriving on any number of detection\n"
"channels, divided into some number of subsets of integration time.\n"
"\n"
"For example, to calculate the intensity of a stream of t2 photons from a\n"
"measurement on a Picoharp, collected in 50ms bins:\n"
"    intensity --bin-width 50000000000 --mode t2 --channels 2\n"
"\n"
"The output format is:\n"
"    time start, time stop, channel 0 counts, channel 1 counts, ...\n"
"\n"
"Because the final photon may not arrive at the end of a time bin, the final\n"
"bin ends at the arrival time of the last photon, permitting post-processing\n"
"to determine whether the effect of that edge is significant.\n"
"\n"
"As an alternative to time bins, all of the photons can be counted by passing\n"
"the flag --count-all. This is useful for normalizing a signal.\n",
		{OPT_HELP, OPT_VERBOSE,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_CHANNELS,
			OPT_BIN_WIDTH, OPT_COUNT_ALL}};

	result = parse_options(argc, argv, &options, &program_options,
			&in_stream, &out_stream);

	if ( ! result ) {
		result = intensity_dispatch(in_stream, out_stream, &options);
	}

	free_options(&options);
	free_streams(in_stream, out_stream);

	return(parse_result(result));	
}
