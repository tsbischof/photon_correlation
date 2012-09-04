#ifndef BIN_INTENSITY_H_
#define BIN_INTENSITY_H_

#include <stdio.h>
#include "limits.h"
#include "options.h"

typedef struct {
	limits_t limits;
	long long *counts;
} bin_counts_t;

int bin_intensity_dispatch(FILE *in_stream, FILE *out_stream, 
		options_t *options);

#endif
