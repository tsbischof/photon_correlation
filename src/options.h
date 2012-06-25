#ifndef OPTIONS_H_
#define OPTIONS_H_

#define QUEUE_SIZE 100000

#include <stdio.h>

#include "histogram_gn.h"

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
	int print_last;
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

int parse_options(int argc, char *argv[], options_t *options, 
		char *options_string);
void usage(int argc, char *argv[], char *options_string);
int is_option(char option, char *options_string);
void free_options(options_t *options);

#endif
