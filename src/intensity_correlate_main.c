#include "correlation/intensity.h"
#include "run.h"
#include "options.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"Calculates the correlation of a multi-channel signal. There are separate \n"
"implementations for linear and logarithmic time scales.\n"
"\n"
"For logarithmic time scales, the multi-tau algorithm is used to approximate\n"
"the result. The accuracy of this method depends on the suitable choice of\n"
"binning and register values. A low ratio of binning to registers increases\n"
"the accuracy of the measurement by making it more akin to a linear\n"
"correlation, at a cost of computational time. \n"
"\n"
"For example, a signal with binning 2 and 4 registers gets transformed into \n"
"the following bins (delays are labeled):\n"
"  | 0| 1| 2| 3|\n"
"  |    0|    2|   4|   6|\n"
"  |          0|        4|      8|      12|\n"
"where the signal filling each vertical column is used to coarsen the \n"
"measurement of the signal with increasing depth. As a result, at a given\n"
"depth s with binning m, the register p represents a delay of:\n"
"                    p*m^s\n"
"Since the first p/m bins are repeated from the previous row, only\n"
"the registers r > p/m are used at any given depth. This leads to a \n"
"total number of bins:\n"
"             bins = (p-p/m)*(s-1)+p\n"
"\n"
"Currently, no linear correlator is implemented.\n"
"\n"
"For all correlators, the output is the same as photon_histogram:\n"
"      (channel 0, channel 1, bin left, bin right, counts)\n"
"except that counts are calculated as double-precision floats.",
		{OPT_VERBOSE, OPT_HELP, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_CHANNELS, OPT_ORDER,
			OPT_TIME_SCALE,
			OPT_BINNING, OPT_REGISTERS, OPT_DEPTH,
			OPT_EOF}};

/* add options to deal with bin width here: scale the time axis appropriately */
	return(run(&program_options, intensity_correlate_dispatch, argc, argv));
}
