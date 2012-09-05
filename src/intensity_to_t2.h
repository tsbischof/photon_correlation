#ifndef INTENSITY_TO_T2_H_
#define INTENSITY_TO_T2_H_

#include <stdio.h>
#include "options.h"
#include "types.h"

typedef struct {
	int64_t time;
	uint32_t counts;
} intensity_t;

int next_intensity(FILE *in_stream, intensity_t *intensity,
		options_t *options);
int intensity_to_t2(FILE *in_stream, FILE *out_stream, options_t *options);

#endif
