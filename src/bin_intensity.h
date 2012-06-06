#ifndef BIN_INTENSITY_H_
#define BIN_INTENSITY_H_

#include <stdio.h>
#include "histogram_gn.h"
#include "t2.h"

typedef struct {
	char *in_filename;
	char *out_filename;
	char *mode_string;
	int mode;
	int channels;
	int queue_size;
	char *time_string;
	limits_t time_limits;
	char *time_scale_string;
	int time_scale;
	char *pulse_string;
	limits_t pulse_limits;
	char *pulse_scale_string;
	int pulse_scale;
} options_t;

typedef struct {
	int channels;
	edges_t *edges;
	long long int **counts;
} counts_t2_t;

void usage(void);

#endif
