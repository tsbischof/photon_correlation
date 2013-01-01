#ifndef INTENSITY_TO_T2_H_
#define INTENSITY_TO_T2_H_

#include <stdio.h>
#include "options.h"
#include "types.h"

typedef struct {
	int64_t time;
	uint32_t counts;
} intensity_t;

int next_intensity(FILE *stream_in, intensity_t *intensity,
		options_t *options);
int intensity_to_t2(FILE *stream_in, FILE *stream_out, options_t *options);

#endif
