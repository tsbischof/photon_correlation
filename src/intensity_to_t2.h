#ifndef INTENSITY_TO_T2_H_
#define INTENSITY_TO_T2_H_

#include <stdio.h>
#include "options.h"
#include "types.h"

typedef struct {
	int64_t time;
	uint32_t counts;
} intensity_t;

int intensity_fread(FILE *stream_in, intensity_t *intensity);
int intensity_fscanf(FILE *stream_in, intensity_t *intensity);
#define INTENSITY_NEXT(x) ( x ? intensity_fread : intensity_fscanf );

typedef int (*intensity_next_t)(FILE *stream_in, intensity_t *intensity);
int intensity_to_t2(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
