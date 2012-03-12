#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include <stdio.h>
#include "histogram_gn.h"

typedef struct {
	char *in_filename;
	char *out_filename;
	char *mode_string;
	int mode;
	char *time_string;
	limits_t time_limits;
	char *pulse_string;
	limits_t pulse_limits;
	int channels;
	int order;
	char *time_scale_string;
	int time_scale;
	char *pulse_scale_string;
	int pulse_scale;
} options_t;

void usage(void);

#endif
