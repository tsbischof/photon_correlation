#ifndef CORRELATE_H_
#define CORRELATE_H_

#include <stdio.h>

typedef struct {
	char *in_filename;
	char *out_filename;
	char *mode_string;
	int mode;
	long long int number;
	int print_every;
	int binary_in;
	int binary_out;
	long long int queue_size;
	long long int max_time_distance;
	long long int max_pulse_distance;
	int order;
	int channels;
} options_t;

void usage(void);

#endif
