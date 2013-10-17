#include "photon_intensity_correlate.h"
#include "run.h"
#include "options.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program performs an intensity correlation of photon arrival time data.\n"
"The algorithm used mimics that of standard multi-tau hardware correlators,\n"
"which perform linear-binned intensity correlations at logarithmically-\n"
"increasing bin widths. For example, with three registers and a binning of\n"
"three, the effective bins look like:\n"
" |0|1|2|\n"
" |     |  5  |  8  | \n"
" |                 |       13        |       22        |\n"
"In this picture, each row forms some number of correlation bins by a normal\n"
"linear correlation of the blank bin with the others. By updating the signal\n"
"by pushing existing values into the larger bins as the small bins are\n"
"filled, we obtain an effective logarithmic scaling, up to some number of\n"
"rescales (the depth). This way, each time bin for the correlation is only\n"
"sampled as often as necessary, ensuring that reasonable signal-to-noise\n"
"can be obtained at all time scales without requiring significant duplication\n"
"of effort.\n",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_CHANNELS, OPT_ORDER,
			OPT_MODE, 
			OPT_START, OPT_STOP,
			OPT_BIN_WIDTH,
			OPT_TIME_SCALE,
			OPT_BINNING, OPT_REGISTERS, OPT_DEPTH,
			OPT_EOF}};

	return(run(&program_options, photon_intensity_correlate_dispatch, 
			argc, argv));
}
