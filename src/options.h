#ifndef OPTIONS_H_
#define OPTIONS_H_

#define QUEUE_SIZE 100000

#include <stdio.h>

#include "limits.h"

typedef struct {
	char short_char;
	char long_char[10];
	char long_name[20];
	char description[1000];
} option_t;

typedef struct {
	char *in_filename;
	FILE *in_stream;
	char *out_filename;
	FILE *out_stream;

	char *mode_string;
	int mode;

	int channels;

	int order;

	int print_every;

	int binary_in;
	int binary_out;

/* Picoquant */
	long long int number;
	int print_header;
	int print_resolution;
	int to_t2;

/* Correlate */
	long long int queue_size;
	long long int max_time_distance;
	long long int min_time_distance;
	long long int max_pulse_distance;
	long long int min_pulse_distance;
	int positive_only;

/* Intensity */
	long long int bin_width;
	int count_all;

/* Histogram */
	char *time_string;
	limits_t time_limits;
	char *pulse_string;
	limits_t pulse_limits;
	
	char *time_scale_string;
	int time_scale;
	char *pulse_scale_string;
	int pulse_scale;
} options_t;

typedef struct {
	int n_options;
	char message[10000];
	int options[100];
} program_options_t;

enum { OPT_HELP, OPT_VERBOSE, OPT_PRINT_EVERY,
		OPT_FILE_IN, OPT_FILE_OUT,
		OPT_MODE, OPT_CHANNELS, OPT_ORDER,
		OPT_BINARY_IN, OPT_BINARY_OUT,
		OPT_PRINT_RESOLUTION, OPT_PRINT_HEADER, OPT_TO_T2, OPT_NUMBER,
		OPT_QUEUE_SIZE, 
		OPT_MAX_TIME_DISTANCE, OPT_MIN_TIME_DISTANCE,
		OPT_MAX_PULSE_DISTANCE, OPT_MIN_PULSE_DISTANCE,
		OPT_POSITIVE_ONLY,
		OPT_BIN_WIDTH, OPT_COUNT_ALL,
		OPT_TIME, OPT_PULSE, OPT_TIME_SCALE, OPT_PULSE_SCALE };

int parse_options(int argc, char *argv[], options_t *options, 
		program_options_t *program_options);
void usage(int argc, char *argv[], 
		program_options_t *program_options);
int is_option(int option, program_options_t *program_options);
char *make_option_string(program_options_t *program_options);
void free_options(options_t *options);
char *get_options_string(program_options_t *program_options);

#endif
