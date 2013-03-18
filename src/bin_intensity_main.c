#include "run.h"
#include "statistics/bin_intensity.h"
#include "options.h"

int main(int argc, char *argv[]) {
	program_options_t program_options = {
"This program calculates the intensities in each bin of a correlation, as \n"
"required for exact normalization of the correlation. The input is either t2\n"
"or t3 data, and the output roughly follows that of the histogram:\n"
"     t1-t0 lower, t1-t0 upper, channel 0 counts, channel 1 counts\n"
"\n"
"For the best results, pass the same arguments for time and pulse bin\n"
"definitions as for histogram.\n",
		{OPT_HELP, OPT_VERBOSE, OPT_VERSION,
			OPT_FILE_IN, OPT_FILE_OUT,
			OPT_MODE, OPT_CHANNELS, OPT_ORDER, 
			OPT_QUEUE_SIZE,
			OPT_TIME, OPT_PULSE,
			OPT_START, OPT_STOP,
			OPT_TIME_SCALE, OPT_PULSE_SCALE, OPT_EOF}};

	return(run(&program_options, bin_intensity, argc, argv));
}
