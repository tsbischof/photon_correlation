/*
 * Copyright (c) 2011-2015, Thomas Bischof
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Massachusetts Institute of Technology nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>

#include "options.h"
#include "error.h"
#include "files.h"
#include "limits.h"
#include "modes.h"

/* 
 * Since there are many small programs which comprise this package, and many
 * of these programs have common options, these routines are used to ensure
 * that options can be handled simply and consistently. The general procedure
 * is as follows: 
 * 1. Specify the help message and the options for the program. 
 * 2. Set the options to their default values.
 * 3. Parse the specified options, checking against the allowed values.
 * 4. Check that the specified options are consistent and valid. 
 */

/*
Currently used:
aAbBcCdDeEfFgGhHiIjJkKmMnNoOpPRsSqQuUvVwWxXyYzZ
Remaining:
lLqQrtT
*/

static pc_option_t pc_options_all[] = {
	{'h', "h", "help", 
			"Prints this usage message."},
	{'v', "v", "version",
			"Print version information."},
	{'V', "V", "verbose", 
			"Print debug-level information."},
	{'p', "p:", "print-every", 
			"Print a status message every n entries.\n"
			"By default, nothing is printed."},
	{'i', "i:", "file-in", 
			"Input filename. By default, this is stdin."},
	{'o', "o:", "file-out", 
			"Output filename. By default, this is stdout."},
	{'m', "m:", "mode", 
			"TTTR mode (t2 or t3) represented by the data."},
	{'c', "c:", "channels",
			"The number of channels in the signal."},
	{'g', "g:", "order",
			"The order of the correlation or histogram."},
	{'G', "G", "use-void",
			"Experimental. Use void pointer arithmetic instead\n"
			"of strong types. This makes for more generic code\n"
			"but runs the risk of introducing strange errors.\n"
			"Use this option at your own risk."},
	{'K', "K:", "seed",
			"Specify the seed for the random number generator."},
	{'q', "q:", "queue-size", 
			"The size of the queue for processing, in number of\n"
			"photons. By default, this is 2^20, and if it is\n"
			"too small a warning message will be displayed, \n"
			"but the queue size will be doubled if possible."},
	{'W', "W:", "window-width",
			"The width of the time bin for processing photons,\n"
			"for a time-dependent calculation. This is used to\n"
			"specify that a calculation should be performed\n"
			"for photons arriving from t to t+dt, repeated\n"
			"for the length of the experiment. The units are\n"
			"picoseconds for t2 mode, and number of pulses\n"
			"for t3."},
	{'f', "f:", "start",
			"The lower limit of time (or pulse) for the run; \n"
			"do not process photons which arrive before \n"
			"this time."},
	{'F', "F:", "stop",
			"The upper limit of time (or pulse) for the run; \n"
			"do not process photons which arrive before \n"
			"this time."},
	{'d', "d:", "max-time-distance", 
			"The maximum time difference between two photons\n"
			"to be considered for the calculation."},
	{'D', "D:", "min-time-distance", 
			"The minimum time difference between two photons\n"
			"to be considered for the calculation."},
	{'e', "e:", "max-pulse-distance", 
			"The maximum pulse difference between two photons\n"
			"to be considered for the calculation."},
	{'E', "E:", "min-pulse-distance", 
			"The maximum pulse difference between two photons\n"
			"to be considered for the calculation."},
	{'P', "P", "positive-only",
			"Process only the positive-time events, that is\n"
			"only photons in their natural time order. This\n"
			"is primarily useful for calculating correlations\n"
			"on a logarithmic scale."},
	{'S', "S", "start-stop",
			"Correlate photons arriving on channel 0 with\n"
			"those arriving on channel 1, in that order.\n"
			"This simulates the behavior of a start-stop\n"
			"correlator as would be implemented by using\n"
			"two signal channels on a two-channel PicoHarp."},
	{'k', "k", "waiting-time",
			"Correlate each photon with the next photon\n"
			"to arrive, to produce the waiting-time\n"
			"distribution for the data."},
	{'w', "w:", "bin-width",
			"The width of the time bin for processing \n"
			"photons. For t2 mode, this is a number of\n"
			"picoseconds, and for t3 mode this is a \n"
			"number of pulses."},
	{'A', "A", "count-all", 
			"Rather than counting photons in a given time bin,\n"
			"count all photons in the stream."},
	{'x', "x:", "time", 
			"The time limits for an axis, following the format:\n"
			"        lower, number of bins, upper \n"
			"with no spaces, and the extrema in picoseconds."},
	{'y', "y:", "pulse",
			"The pulse limits for an axis, following the format:\n"
			"        lower, number of bins, upper \n"
			"with no spaces, and the extrema in picoseconds."},
	{'X', "X:", "time-scale",
			"The scale of the time axis, one of:\n"
			"   linear: linear interpolation between limits\n"
			"      log: logarithmic interpolation (linear\n"
			"           on a log axis)\n"
			" log-zero: same as log, except that the lowest\n"
			"           bin is extended to include 0"},
	{'Y', "Y:", "pulse-scale",
			"The scale of the pulse axis, one of:\n"
			"   linear: linear interpolation between limits\n"
			"      log: logarithmic interpolation (linear\n"
			"           on a log axis)\n"
			" log-zero: same as log, except that the lowest\n"
			"           bin is extended to include 0"},
	{'Q', "Q:", "intensity",
			"The limits for an intensity axis in e.g. a FLID\n"
			"diagram or time-dependent gn. Follows the format:\n"
			"         lower, number of bins, upper \n"
			"with no spaces. The lower and upper limits \n"
			"are the number of photons found in a given time\n"
			"window, and the number of bins must evenly divide\n"
			"the span of counts ((upper - lower) %% bins == 0)."},
	{'u', "u:", "time-offsets",
			"A common-delimited list of time offsets to\n"
			"apply to the channels. All channels must be\n"
			"represented, even if the offset is 0."},
	{'U', "U:", "pulse-offsets", 
			"A comma-delimited list of pulse offsets to \n"
			"apply to the channels. All channels must be\n"
			"represented, even if the offset is 0."},
	{'s', "s:", "suppress",
			"A comma-delmited list of channels to remove\n"
			"from the stream."},
	{'Z', "Z", "exact-normalization",
			"Rather than using intensity to calculate the\n"
			"average intensity for normalization, use\n"
			"bin_intensity to get the exact result for each\n"
			"histogram bin."},
	{'R', "R:", "repetition-rate",
			"Specifies the repetition rate for the sync pulse."},
	{'O', "O:", "time-origin",
			"Specifies the origin in time for the photons being\n"
			"converted. This compensates for the fact that t2\n"
			"measurements are not necessarily synchronized with\n"
			"the excitation source."},
	{'M', "M:", "convert",
			"Specifies the output mode and style for converting\n"
			"photons between types. Choices are:\n"
			"t2: use the sync rate to form t2 times\n"
			"as-t2: use the pulse number as t2 time\n"
			"t3: use the t2 time and sync rate to get pulse\n"
			"    number, time"},
	{'z', "z:", "copy-to-channel",
			"Copies all valid photons to the specified channel.\n"
			"Use this to create a summed autocorrelation across\n"
			"many channels."},
	{'B', "B:", "binning",
			"Number of bins wide to use for the multi-tau\n"
			"correlation. For more accurate results, lower the\n"
			"ratio of binning to registers."},
	{'a', "a:", "registers",
			"Number of registers to use for the multi-tau\n"
			"correlation. For more accurate results, lower the\n"
			"ratio of binning to registers."},
	{'b', "b:", "depth",
			"Number of levels deep for the multi-tau \n"
			"correlation. The maximum delay in units of time\n"
			"is: binning^(depth-1)*(registers-1)"},
	{'C', "C", "correlate-successive",
			"Instead of returning photons with their original\n"
			"timing, return their timing relative the the\n"
			"previous photon from their pulse."},
	{'H', "H", "filter-afterpulsing",
			"For t3 mode, retain only the first photon on a\n"
			"channel for a given pulse."},
	{'I', "I:", "time-gating",
			"For t3 mode, suppress any photons which arrive\n"
			"before the specified time after a pulse."},
	{'j', "j:", "sync-channel",
			"Specifies the t2 channel which should be treated\n"
			"as the sync source for conversion to t3 photons."},
	{'J', "J:", "sync-divider",
			"Specifies the sync divider, which effectively tells\n"
			"how many sync events are skipped between recorded\n"
			"events."},
	{'n', "n:", "threshold",
			"The minimum number of photons in a window to be\n"
			"considered for calculation."},
	{'N', "N:", "time-threshold",
			"The time dividing early and late arrivals, in ps."},
	};


static struct option pc_options_long[] = {
	{"help", no_argument, 0, 'h'},
	{"verbose", no_argument, 0, 'V'},
	{"version", no_argument, 0, 'v'},
	{"print-every", required_argument, 0, 'p'},

	{"file-in", required_argument, 0, 'i'},
	{"file-out", required_argument, 0, 'o'},

	{"mode", required_argument, 0, 'm'},
	{"channels", required_argument, 0, 'c'},
	{"order", required_argument, 0, 'g'},

	{"use-void", no_argument, 0, 'G'},
	{"seed", required_argument, 0, 'K'},

	{"window-width", required_argument, 0, 'W'},

/* Correlate */
	{"queue-size", required_argument, 0, 'q'},
	{"max-time-distance", required_argument, 0, 'd'},
	{"min-time-distance", required_argument, 0, 'D'},
	{"max-pulse-distance", required_argument, 0, 'e'},
	{"min-pulse-distance", required_argument, 0, 'E'},
	{"positive-only", no_argument, 0, 'P'},
	{"start-stop", no_argument, 0, 'S'},
	{"waiting-time", no_argument, 0, 'k'},

/* Intensity */ 
	{"bin-width", required_argument, 0, 'w'},
	{"count-all", no_argument, 0, 'A'},
	{"start", required_argument, 0, 'f'},
	{"stop", required_argument, 0, 'F'},

/* Histogram */ 
	{"time", required_argument, 0, 'x'},
	{"pulse", required_argument, 0, 'y'},
	{"time-scale", required_argument, 0, 'X'},
	{"pulse-scale", required_argument, 0, 'Y'},
	{"intensity", required_argument, 0, 'Q'},

/* Temper */
	{"time-offsets", required_argument, 0, 'u'},
	{"pulse-offsets", required_argument, 0, 'U'},
	{"suppress", required_argument, 0, 's'},
	{"filter-afterpulsing", no_argument, 0, 'H'},
	{"time-gating", required_argument, 0, 'I'},

/* correlate_vector */ 
	{"approximate", required_argument, 0, 'B'},
	{"true-correlation", no_argument, 0, 'C'},

/* gn */
	{"exact-normalization", no_argument, 0, 'Z'},

/* photons */
	{"repetition-rate", required_argument, 0, 'R'},
	{"time-origin", required_argument, 0, 'O'},
	{"convert", required_argument, 0, 'M'},
	{"copy-to-channel", required_argument, 0, 'z'},

/* correlate intensity */
	{"binning", required_argument, 0, 'B'},
	{"registers", required_argument, 0, 'a'},
	{"depth", required_argument, 0, 'b'},

/* number to channel */
	{"correlate-successive", no_argument, 0, 'C'},

/* synced t2 */
	{"sync-channel", required_argument, 0, 'j'},
	{"sync-divider", required_argument, 0, 'J'},

/* threshold */
	{"threshold", required_argument, 0, 'n'},

/* time threshold */
	{"time-threshold", required_argument, 0, 'N'},

	{0, 0, 0, 0}};



pc_options_t *pc_options_alloc(void) {
	return(malloc(sizeof(pc_options_t)));
}

void pc_options_init(pc_options_t *options, 
		program_options_t *program_options) {
	options->program_options = program_options;
	pc_options_default(options);
	pc_options_make_string(options);
}

void pc_options_free(pc_options_t **options) {
	if ( *options != NULL ) {
		free((*options)->filename_in);
		free((*options)->filename_out);
		free((*options)->mode_string);
		free((*options)->time_string);
		free((*options)->pulse_string);
		free((*options)->time_scale_string);
		free((*options)->pulse_scale_string);
		free((*options)->time_offsets_string);
		free((*options)->time_offsets);
		free((*options)->pulse_offsets_string);
		free((*options)->pulse_offsets);
		free((*options)->convert_string);
		free(*options);
		*options = NULL;
	}
}

void pc_options_default(pc_options_t *options) {
	options->usage = false;
	options->verbose = false;
	options->version = false;

	options->filename_in = NULL;
	options->filename_out = NULL;

	options->mode_string = NULL;
	options->mode = MODE_UNKNOWN;

	options->channels = 2;
	options->order = 2;
	
	options->print_every = 0;

	options->use_void = false;
	options->seed = 0xDEADBEEF;

	options->window_width = 0;

	options->queue_size = QUEUE_SIZE;
	options->max_time_distance = 0;
	options->min_time_distance = 0;
	options->max_pulse_distance = 0;
	options->min_pulse_distance = 0;
	options->positive_only = false;
	options->start_stop = false;
	options->waiting_time = false;

	options->bin_width = 0;
	options->count_all = false;
	options->set_start = false;
	options->start = 0;
	options->set_stop = false;
	options->stop = 0;

	options->time_string = NULL;
	options->pulse_string = NULL;
	options->intensity_string = NULL;

	options->time_scale_string = NULL;
	options->time_scale = SCALE_LINEAR;
	options->pulse_scale_string = NULL;
	options->pulse_scale = SCALE_LINEAR;

	options->suppress_channels = false;
	options->suppress_string = NULL;
	options->suppressed_channels = NULL;

	options->offset_time = false;
	options->time_offsets_string = NULL;
	options->time_offsets = NULL;

	options->offset_pulse = false;
	options->pulse_offsets_string = NULL;
	options->pulse_offsets = NULL;

	options->filter_afterpulsing = false;

	options->time_gating = false;
	options->gate_time = 0;

	options->exact_normalization = false;

	options->repetition_rate = 0;
	options->time_origin = 0;
	options->convert_string = NULL;
	options->convert = MODE_UNKNOWN;

	options->copy_to_channel = false;
	options->copy_to_this_channel = 0;

	options->binning = 2;
	options->registers = 16;
	options->depth = 32;

	options->correlate_successive = false;

	options->sync_channel = 0;
	options->sync_divider = 0;

	options->threshold = 0;
	options->time_threshold = 0;
}

int pc_options_valid(pc_options_t const *options) {
	if ( options->usage || options->version ) {
		return(false);
	}

	if ( pc_options_has_option(options, OPT_CHANNELS) 
			&& options->channels < 1 ) {
		error("Must have at least 1 channel (%d specified).\n", 
				options->channels);
		return(false);
	}

	if ( pc_options_has_option(options, OPT_ORDER) && options->order < 1 ) {
		error("Order of correlation/histogram must be at least 1 (%d "
				"specified).", options->order);
		return(false);
	}

	if ( pc_options_has_option(options, OPT_MODE) &&
			options->mode == MODE_UNKNOWN ) {
		error("Invalid mode: %d\n", options->mode);
		return(false);
	}

	if ( pc_options_has_option(options, OPT_TIME_SCALE) &&
			options->time_scale == SCALE_UNKNOWN ) {
		error("Unknown time scale.\n");
		return(false);
	}

	if ( pc_options_has_option(options, OPT_PULSE_SCALE) &&
			options->pulse_scale == SCALE_UNKNOWN ) {
		error("Unknown pulse scale.\n");
		return(false);
	}

	if ( pc_options_has_option(options, OPT_TIME) &&
			! limits_valid(&(options->time_limits)) ) {
		error("Invalid time limits.\n");
		return(false);
	}

	if ( pc_options_has_option(options, OPT_PULSE) 
			&& options->mode == MODE_T3 
			&& options->order > 1
			&& ! limits_valid(&(options->pulse_limits)) ) {
		error("Invalid pulse limits.\n");
		return(false);
	}
		
	if ( pc_options_has_option(options, OPT_START_STOP) && 
			options->start_stop && 
			(options->channels != 2 || 
			 options->order != 2 || 
			 options->mode != MODE_T2 ) ) {
		error("Start-stop mode is only defined for 2 channels, "
				"t2 mode, and order 2.\n");
		return(false);
	}

	if ( pc_options_has_option(options, OPT_TIME_OFFSETS) &&
			options->offset_time &&
			! offsets_valid(options->time_offsets) ) {
		error("Invalid time offsets.\n");
		return(false);
	}

	if ( pc_options_has_option(options, OPT_PULSE_OFFSETS) &&
			options->offset_pulse &&
			! offsets_valid(options->pulse_offsets) ) {
		error("Invalid pulse offsets.\n");
		return(false);
	}

	if ( pc_options_has_option(options, OPT_TIME_GATING)
			&& options->mode != MODE_T3 ) {
		error("Time gating only defined for t3 mode.\n");
		return(false);
	}

	return(true);
}

int pc_options_parse(pc_options_t *options, 
		int const argc, char * const *argv) { 
	int c = 0;
	int option_index = 0;
	char const *options_string = pc_options_string(options);

	while ( (c = getopt_long(argc, argv, options_string,
						pc_options_long, &option_index)) != -1 ) {
		if ( strchr(options_string, c) == NULL && c != '?' ) {
			error("Unknown option %c\n", c);
			c = '?';
		}
			
		switch (c) {
			case 'h':
				options->usage = true;
				break;
			case 'V':
				options->verbose = true;
				verbose = 1;
				break;
			case 'v':
				options->version = true;
				break;
			case 'p':
				options->print_every = strtol(optarg, NULL, 10);
				break;
			case 'i':
				options->filename_in = strdup(optarg);
				break;
			case 'o':
				options->filename_out = strdup(optarg);
				break;
			case 'm':
				options->mode_string = strdup(optarg);
				break;
			case 'c':
				options->channels = strtol(optarg, NULL, 10);
				break;
			case 'g':
				options->order = strtol(optarg, NULL, 10);
				break;
			case 'G':
				options->use_void = true;
				break;
			case 'K':
				options->seed = strtoul(optarg, NULL, 10);
				break;
			case 'q':
				options->queue_size = strtoul(optarg, NULL, 10);
				break;
			case 'W':
				options->window_width = strtoull(optarg, NULL, 10);
				break;
			case 'd':
				options->max_time_distance = strtoull(optarg, NULL, 10);
				break;
			case 'D':
				options->min_time_distance = strtoull(optarg, NULL, 10);
				break;
			case 'e':
				options->max_pulse_distance = strtoull(optarg, NULL, 10);
				break;
			case 'E':
				options->min_time_distance = strtoull(optarg, NULL, 10);
				break;
			case 'P':
				options->positive_only = true;
				break;
			case 'w':
				options->bin_width = strtoull(optarg, NULL, 10);
				break;
			case 'A':
				options->count_all = true;
				break;
			case 'x':
				options->time_string = strdup(optarg);
				break;
			case 'y':
				options->pulse_string = strdup(optarg);
				break;
			case 'X':
				options->time_scale_string = strdup(optarg);
				break;
			case 'Y':
				options->pulse_scale_string = strdup(optarg);
				break;
			case 'Q':
				options->intensity_string = strdup(optarg);
				break;
			case 'S':
				options->start_stop = true;
				break;
			case 'k':
				options->waiting_time = true;
				break;
			case 'f':
				options->set_start = true;
				options->start = strtoll(optarg, NULL, 10);
				break;
			case 'F':
				options->set_stop = true;
				options->stop = strtoll(optarg, NULL, 10);
				break;
			case 'u':
				options->offset_time = true;
				options->time_offsets_string = strdup(optarg);
				break;
			case 'U':
				options->offset_pulse = true;
				options->pulse_offsets_string = strdup(optarg);
				break;
			case 's':
				options->suppress_channels = true;
				options->suppress_string = strdup(optarg);
				break;
			case 'Z':
				options->exact_normalization = true;
				break;
			case 'R':
				options->repetition_rate = strtof(optarg, NULL);
				break;
			case 'O':
				options->time_origin = strtoll(optarg, NULL, 10);
				break;
			case 'M':
				options->convert_string = strdup(optarg);
				pc_options_parse_convert(options);
				break;
			case 'z':
				options->copy_to_channel = true;
				options->copy_to_this_channel = strtoul(optarg, NULL, 10);
				break;
			case 'B':
				options->binning = strtoul(optarg, NULL, 10);
				break;
			case 'a':
				options->registers = strtoul(optarg, NULL, 10);
				break;
			case 'b':
				options->depth = strtoul(optarg, NULL, 10);
				break;
			case 'C':
				options->correlate_successive = true;
				break;
			case 'H':
				options->filter_afterpulsing = true;
				break;
			case 'I':
				options->time_gating = true;
				options->gate_time = strtoll(optarg, NULL, 10);
				break;
			case 'j':
				options->sync_channel = strtoul(optarg, NULL, 10);
				break;
			case 'J':
				options->sync_divider = strtoul(optarg, NULL, 10);
				break;
			case 'n':
				options->threshold = strtoul(optarg, NULL, 10);
				break;
			case 'N':
				options->time_threshold = strtoull(optarg, NULL, 10);
				break;
			case '?':
			default:
				options->usage = true;
				break;
		}
	}

	if ( options->usage || options->version ) {
		return(PC_USAGE);
	}

	if ( pc_options_has_option(options, OPT_MODE) &&
			pc_options_parse_mode(options) != PC_SUCCESS ) {
		error("Invalid mode: %d\n", options->mode);
		return(PC_ERROR_OPTIONS);
	}

	if ( pc_options_has_option(options, OPT_TIME_SCALE) &&
			pc_options_parse_time_scale(options) != PC_SUCCESS ) {
		return(PC_ERROR_OPTIONS);
	}

	if ( pc_options_has_option(options, OPT_PULSE_SCALE) &&
			pc_options_parse_pulse_scale(options) != PC_SUCCESS ) {
		return(PC_ERROR_OPTIONS);
	}

	if ( pc_options_has_option(options, OPT_TIME) &&
			pc_options_parse_time_limits(options) != PC_SUCCESS ) {
		return(PC_ERROR_OPTIONS);
	}

	if ( pc_options_has_option(options, OPT_PULSE) 
			&& options->mode == MODE_T3 
			&& options->order > 1
			&& pc_options_parse_pulse_limits(options) != PC_SUCCESS ) {
		return(PC_ERROR_OPTIONS);
	}
		
	if ( pc_options_has_option(options, OPT_START_STOP) && 
			options->start_stop && 
			(options->channels != 2 || 
			 options->order != 2 || 
			 options->mode != MODE_T2 ) ) {
		error("Start-stop mode is only defined for 2 channels, "
				"t2 mode, and order 2.\n");
		return(PC_ERROR_OPTIONS);
	}

	if ( pc_options_has_option(options, OPT_SUPPRESS) &&
			pc_options_parse_suppress(options) != PC_SUCCESS ) {
		return(PC_ERROR_OPTIONS);
	}

	if ( pc_options_has_option(options, OPT_TIME_OFFSETS) &&
			pc_options_parse_time_offsets(options) != PC_SUCCESS ) {
		return(PC_ERROR_OPTIONS);
	}

	if ( pc_options_has_option(options, OPT_PULSE_OFFSETS) &&
			pc_options_parse_pulse_offsets(options) != PC_SUCCESS ) {
		return(PC_ERROR_OPTIONS);
	}

	if ( pc_options_has_option(options, OPT_INTENSITY) &&
			pc_options_parse_intensity_limits(options) != PC_SUCCESS ) {
		return(PC_ERROR_OPTIONS);
	}

	return(PC_SUCCESS);
}


int pc_options_parse_mode(pc_options_t *options) {
	return(mode_parse(&(options->mode), options->mode_string));
}

int pc_options_parse_time_scale(pc_options_t *options) {
	return(scale_parse(&(options->time_scale), options->time_scale_string));
}

int pc_options_parse_pulse_scale(pc_options_t *options) {
	return(scale_parse(&(options->pulse_scale), options->pulse_scale_string));
}

int pc_options_parse_time_limits(pc_options_t *options) {
	return(limits_parse(&(options->time_limits), options->time_string));
}

int pc_options_parse_pulse_limits(pc_options_t *options) {
	return(limits_parse(&(options->pulse_limits), 
			options->pulse_string));
}

int pc_options_parse_intensity_limits(pc_options_t *options) {
	return(limits_int_parse(&(options->intensity_limits),
			options->intensity_string));
}

int pc_options_parse_suppress(pc_options_t *options) {
	return(suppress_parse(options));
}

int pc_options_parse_time_offsets(pc_options_t *options) {
	if ( options->offset_time ) {
		return(offsets_parse(&(options->time_offsets), 
				options->time_offsets_string,
				options->channels));
	} else {
		return(PC_SUCCESS);
	}
}

int pc_options_parse_pulse_offsets(pc_options_t *options) {
	if ( options->offset_pulse ) {
		return(offsets_parse(&(options->pulse_offsets),
				options->pulse_offsets_string,
				options->channels));
	} else {
		return(PC_SUCCESS);
	}
}

int pc_options_parse_convert(pc_options_t *options) {
	return(mode_parse(&(options->convert), options->convert_string));
}

char const* pc_options_string(pc_options_t const *options) {
	return(&(options->string[0]));
}

void pc_options_make_string(pc_options_t *options) {
	pc_option_t *option;
	int i;
	int j;
	int index = 0;
	
	for ( i = 0; options->program_options->options[i] != OPT_EOF; i++ ) {
		option = &pc_options_all[options->program_options->options[i]];

		for ( j = 0; j < strlen(option->long_char); j++ ) {
			options->string[index++] = option->long_char[j];
		}
	}
	options->string[index] = '\0';
}

int pc_options_has_option(pc_options_t const *options, int option) {
	int i;

	for ( i = 0; options->program_options->options[i] != OPT_EOF; i++ ) {
		if ( option == options->program_options->options[i] ) {
			return(1);
		}
	}

	return(0);
}


void pc_options_usage(pc_options_t const *options, 
		int const argc, char * const *argv) {
	int i,j;
	pc_option_t *option;

	fprintf(stderr, "Usage: %s [options]\n\n", argv[0]);
	pc_options_version(options, argc, argv);
	fprintf(stderr, "\n");

	for ( i = 0; options->program_options->options[i] != OPT_EOF; i++ ) {
		option = &pc_options_all[options->program_options->options[i]];

		fprintf(stderr, "%*s-%c, --%s: ", 
				20-(int)strlen(option->long_name),
				"",
				option->short_char,
				option->long_name);

		for ( j = 0; j < strlen(option->description); j++ ) {
			if ( option->description[j] == '\n' ) {
				fprintf(stderr, "\n%*s", 28, " ");
			} else {
				fprintf(stderr, "%c", option->description[j]);
			}
		} 

		fprintf(stderr, "\n");
	}

	fprintf(stderr, "\n%s\n", options->program_options->message);
}

void pc_options_version(pc_options_t const *options, 
		int const argc, char * const *argv) {
/*	fprintf(stderr, 
		"%s v%s (build %s)\n", 
		argv[0], 
		STR(VERSION),
		STR(VERSION_STRING)); */
	fprintf(stderr, 
		"%s v%s\n", 
		argv[0], 
		VERSION);
}

int pc_options_fprintf(FILE *stream_out, pc_options_t const *options) {
	int i;

	fprintf(stream_out, "usage = %d\n", options->usage);
	fprintf(stream_out, "verbose = %d\n", options->verbose);
	fprintf(stream_out, "version = %d\n", options->version);
	fprintf(stream_out, "filename_in = %s\n", options->filename_in);
	fprintf(stream_out, "filename_out = %s\n", options->filename_out);
	fprintf(stream_out, "mode = %d (%s)\n", 
			options->mode, options->mode_string);
	fprintf(stream_out, "channels = %d\n", options->channels);
	fprintf(stream_out, "order = %d\n", options->order);
	fprintf(stream_out, "print_every = %d\n", options->print_every);
	fprintf(stream_out, "seed = 0x%x\n", options->seed);
	fprintf(stream_out, "queue_size = %zu\n", options->queue_size);
	fprintf(stream_out, "window_width = %llu\n", options->window_width);

	fprintf(stream_out, "max_time_distance = %llu\n", 
			options->max_time_distance);
	fprintf(stream_out, "min_time_distance = %llu\n", 
			options->min_time_distance);
	fprintf(stream_out, "max_pulse_distance = %llu\n", 
			options->max_pulse_distance);
	fprintf(stream_out, "min_pulse_distance = %llu\n", 
			options->min_pulse_distance);
	fprintf(stream_out, "positive_only = %d\n", options->positive_only);
	fprintf(stream_out, "start_stop = %d\n", options->start_stop);

	fprintf(stream_out, "bin_width = %llu\n", options->bin_width);
	fprintf(stream_out, "count_all = %d\n", options->count_all);
	fprintf(stream_out, "set_start = %d\n", options->set_start);
	fprintf(stream_out, "start = %lld\n", options->start);
	fprintf(stream_out, "set_stop = %d\n", options->set_stop);
	fprintf(stream_out, "stop = %lld\n", options->stop);

	fprintf(stream_out, "time_limits = %lf,%zu,%lf (%s)\n",
			options->time_limits.lower,
			options->time_limits.bins,
			options->time_limits.upper,
			options->time_string);
	fprintf(stream_out, "pulse_limits = %lf,%zu,%lf (%s)\n",
			options->pulse_limits.lower,
			options->pulse_limits.bins,
			options->pulse_limits.upper,
			options->pulse_string);
	fprintf(stream_out, "time_scale = %d (%s)\n", 
			options->time_scale, options->time_scale_string);
	fprintf(stream_out, "pulse_scale = %d (%s)\n", 
			options->pulse_scale, options->pulse_scale_string);
	fprintf(stream_out, "intensity_limits = %lld,%zu,%lld (%s)\n",
			options->intensity_limits.lower,
			options->intensity_limits.bins,
			options->intensity_limits.upper,
			options->intensity_string);

	fprintf(stream_out, "suppress_channels = %d\n", options->suppress_channels);
	
	fprintf(stream_out, "suppressed_channels = ");
	if ( options->suppressed_channels != NULL ) {
		for ( i = 0; i < options->channels; i++ ) {
			fprintf(stream_out, "%d", options->suppressed_channels[i]);
			if ( i + 1 != options->channels ) {
				fprintf(stream_out, ",");
			}
		}
	}
	fprintf(stream_out, "(%s)\n", options->suppress_string);

	fprintf(stream_out, "gate time = %d (%lld)\n", 
			options->time_gating, options->gate_time);

	fprintf(stream_out, "time_offsets = ");
	if ( options->offset_time ) {
		for ( i = 0; i < options->channels; i++ ) {
			fprintf(stream_out, "%lld", options->time_offsets[i]);
			if ( i + 1 != options->channels ) {
				fprintf(stream_out, ",");
			}
		}
	}
	fprintf(stream_out, "(%s)\n", options->time_offsets_string);

	fprintf(stream_out, "pulse_offsets = ");
	if ( options->offset_pulse ) {
		for ( i = 0; i < options->channels; i++ ) {
			fprintf(stream_out, "%lld", options->pulse_offsets[i]);
			if ( i + 1 != options->channels ) {
				fprintf(stream_out, ",");
			}
		}
	}
	fprintf(stream_out, "(%s)\n", options->pulse_offsets_string);


	fprintf(stream_out, "exact_normalization = %d\n", 
			options->exact_normalization);

	fprintf(stream_out, "repetition_rate = %lf\n", options->repetition_rate);
	fprintf(stream_out, "convert = %d (%s)\n", 
			options->convert, options->convert_string);

	fprintf(stream_out, "correlate_successive = %d\n", 
			options->correlate_successive);

	fprintf(stream_out, "sync_channel = %u\n", options->sync_channel);
	fprintf(stream_out, "sync_divider = %u\n", options->sync_divider);

	fprintf(stream_out, "threshold = %u\n", options->threshold);

	fprintf(stream_out, "time_threshold = %llu\n", options->time_threshold);

	return( ferror(stream_out) ? PC_ERROR_IO : PC_SUCCESS );
}

int offsets_parse(long long **offsets, char *offsets_string, 
		int const channels) {
	char *c;
	int channel;

	debug("Parsing offsets from %s\n", offsets_string);

	*offsets = (long long *)malloc(sizeof(long long)*channels);

	if ( *offsets == NULL ) {
		error("Could not allocate memory for offsets\n");
		return(PC_ERROR_MEM);
	}

	c = strtok(offsets_string, ",");

	for ( channel = 0; channel < channels; channel++ ) {
		if ( c == NULL ) {
			error("Not enough offsets specified (%d found)\n", channel);
			return(PC_ERROR_OPTIONS);
		} else {
			(*offsets)[channel] = strtoull(c, NULL, 10);
			debug("Offset for channel %d: %"PRId64"\n",
					channel, (*offsets)[channel]);
		}

		c = strtok(NULL, ",");
	}
		
	return(PC_SUCCESS);
}

int offsets_valid(long long const *offsets) {
	return(offsets != NULL);
}

int suppress_parse(pc_options_t *options) {
	char *c;
	int channel;
	
	if ( options->suppress_channels ) {
		options->suppressed_channels = (int *)malloc(
				sizeof(int)*options->channels);

		if ( options->suppressed_channels == NULL ) {
			error("Could not allocate memory for supressed channels.\n");
			return(PC_ERROR_MEM);
		}

		for ( channel = 0; channel < options->channels; channel++ ) {
			options->suppressed_channels[channel] = 0;
		}

		c = strtok(options->suppress_string, ",");

		while ( c != NULL ) {
			channel = strtol(c, NULL, 10);

			if ( channel == 0 && strcmp(c, "0") ) {
				/* check for parse errors */
				error("Invalid channel to suppress: %s\n", c);
				return(PC_ERROR_OPTIONS);
			} else if ( channel < 0 || channel >= options->channels ) {
				error("Invalid channel to suppress: %d\n", channel);
				return(PC_ERROR_OPTIONS);
			} else {
				debug("Suppressing channel %d\n", channel);
				if ( options->suppressed_channels[channel] ) {
					warn("Duplicate suppression of channel %d\n", channel);
				}
				options->suppressed_channels[channel] = 1;
			}

			c = strtok(NULL, ",");
		}
	}

	return(PC_SUCCESS);
}

int suppress_valid(pc_options_t const *options) {
	return(options->suppressed_channels != NULL);
}

