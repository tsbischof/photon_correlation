#ifndef INTENSITY_H_
#define INTENSITY_H_

#include <stdio.h>

#include "photon.h"
#include "options.h"

typedef struct {
	int channels;
	window_t window;
	uint64_t *counts;
} counts_t;

int intensity_dispatch(FILE *stream_in, FILE *stream_out, options_t *options);

counts_t *counts_alloc(int channels);
void counts_init(counts_t *counts);
void counts_free(counts_t **counts);
int counts_increment(counts_t *counts, int channel);

typedef int (*counts_next_t)(FILE *stream_in, counts_t *counts);
typedef int (*counts_print_t)(FILE *stream_out, counts_t const *counts);

int counts_fscanf(FILE *stream_in, counts_t *counts);
int counts_fprintf(FILE *stream_out, counts_t const *counts);

int counts_echo(FILE *stream_in, FILE *stream_out, int channels);

#endif
