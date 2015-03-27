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

#ifndef OPTIONS_H_
#define OPTIONS_H_

#define QUEUE_SIZE 1024*1024

#include <stdio.h>
#include "types.h"

#include "limits.h"

typedef struct {
	char short_char;
	char long_char[10];
	char long_name[50];
	char description[1000];
} pc_option_t;

typedef struct {
	char message[10000];
	int options[100];
} program_options_t;

typedef struct {
	program_options_t *program_options;
	char string[1024];

	int usage;
	int verbose;
	int version;

	char *filename_in;
	char *filename_out;

	char *mode_string;
	int mode;

	int channels;
	int order;

	int print_every;

	int use_void;
	unsigned int seed;

	unsigned long long window_width;

/* Correlate */
	size_t queue_size;
	unsigned long long max_time_distance;
	unsigned long long min_time_distance;
	unsigned long long max_pulse_distance;
	unsigned long long min_pulse_distance;
	int positive_only;
	int start_stop;
	int waiting_time;

/* Intensity */
	unsigned long long bin_width;
	int count_all;
	int set_start;
	long long start;
	int set_stop;
	long long stop;

/* Histogram */
	char *time_string;
	limits_t time_limits;
	char *pulse_string;
	limits_t pulse_limits;
	
	char *time_scale_string;
	int time_scale;
	char *pulse_scale_string;
	int pulse_scale;

	char *intensity_string;
	limits_int_t intensity_limits;

/* temper */
	int suppress_channels;
	char *suppress_string;
	int *suppressed_channels;

	int offset_time;
	char *time_offsets_string;
	long long *time_offsets;

	int offset_pulse;
	char *pulse_offsets_string;
	long long *pulse_offsets;

	int filter_afterpulsing;

	int time_gating;
	long long gate_time;

/* gn */
	int exact_normalization;

/* photons */
	double repetition_rate;
	long long time_origin;
	char *convert_string;
	int convert;
	int copy_to_channel;
	unsigned int copy_to_this_channel;

/* correlate intensity */
	unsigned int binning;
	unsigned int registers;
	unsigned int depth;

/* number to channels */
	int correlate_successive;

/* synced t2 */
	unsigned int sync_channel;
	unsigned int sync_divider;

/* threshold */
	unsigned int threshold;

/* time threshold */
	unsigned long long time_threshold;
} pc_options_t;

enum { OPT_HELP, OPT_VERSION,
		OPT_VERBOSE, OPT_PRINT_EVERY,
		OPT_FILE_IN, OPT_FILE_OUT,
		OPT_MODE, OPT_CHANNELS, OPT_ORDER,
		OPT_USE_VOID, OPT_SEED,
		OPT_QUEUE_SIZE, 
		OPT_WINDOW_WIDTH,
		OPT_START, OPT_STOP,
		OPT_MAX_TIME_DISTANCE, OPT_MIN_TIME_DISTANCE,
		OPT_MAX_PULSE_DISTANCE, OPT_MIN_PULSE_DISTANCE,
		OPT_POSITIVE_ONLY, OPT_START_STOP,
		OPT_WAITING_TIME,
		OPT_BIN_WIDTH, OPT_COUNT_ALL,
		OPT_TIME, OPT_PULSE, OPT_TIME_SCALE, OPT_PULSE_SCALE,
		OPT_INTENSITY,
		OPT_TIME_OFFSETS, OPT_PULSE_OFFSETS, 
		OPT_SUPPRESS,
		OPT_EXACT_NORMALIZATION, 
		OPT_REPETITION_TIME, OPT_TIME_ORIGIN,
		OPT_CONVERT, OPT_COPY_TO_CHANNEL,
		OPT_BINNING, OPT_REGISTERS, OPT_DEPTH,
		OPT_CORRELATE_SUCCESSIVE,
		OPT_FILTER_AFTERPULSING,
		OPT_TIME_GATING,
		OPT_SYNC_CHANNEL, OPT_SYNC_DIVIDER,
		OPT_THRESHOLD,
		OPT_TIME_THRESHOLD,
		OPT_EOF };

pc_options_t *pc_options_alloc(void);
void pc_options_init(pc_options_t *options,
		program_options_t *program_options);
void pc_options_free(pc_options_t **options);

void pc_options_default(pc_options_t *options);
int pc_options_valid(pc_options_t const *options);
int pc_options_parse(pc_options_t *options, 
		int const argc, char * const *argv);

int pc_options_parse_mode(pc_options_t *options);
int pc_options_parse_time_scale(pc_options_t *options);
int pc_options_parse_pulse_scale(pc_options_t *options);
int pc_options_parse_time_limits(pc_options_t *options);
int pc_options_parse_pulse_limits(pc_options_t *options);
int pc_options_parse_intensity_limits(pc_options_t *options);
int pc_options_parse_suppress(pc_options_t *options);
int pc_options_parse_time_offsets(pc_options_t *options);
int pc_options_parse_pulse_offsets(pc_options_t *options);
int pc_options_parse_convert(pc_options_t *options);

void pc_options_usage(pc_options_t const *options, 
		int const argc, char * const *argv);
void pc_options_version(pc_options_t const *options,
		int const argc, char * const *argv);

char const* pc_options_string(pc_options_t const *options);
void pc_options_make_string(pc_options_t *options);
int pc_options_has_option(pc_options_t const *options, int const option);
int pc_options_fprintf(FILE *stream_out, pc_options_t const *options);

int offsets_parse(long long **offsets, char *offsets_string,
		int const channels);
int offsets_valid(long long const *offsets);

int suppress_parse(pc_options_t *options);
int suppress_valid(pc_options_t const *options);

#endif
