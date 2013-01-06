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
#define COUNTS_NEXT(x) ( x ? counts_fread : counts_fscanf );
#define COUNTS_PRINT(x) ( x ? counts_fwrite : counts_fprintf );

int counts_fread(FILE *stream_in, counts_t *counts);
int counts_fscanf(FILE *stream_in, counts_t *counts);
int counts_fprintf(FILE *stream_out, counts_t const *counts);
int counts_fwrite(FILE *stream_out, counts_t const *counts);

int counts_echo(FILE *stream_in, FILE *stream_out, 
		int binary_in, int binary_out, int channels);

/* Legacy code */
counts_t *allocate_counts(int channels);
void init_counts(counts_t *counts);
void free_counts(counts_t **counts);
int increment_counts(counts_t *counts, int channel);
void print_counts(FILE *stream_out, counts_t *counts, options_t *options);
int next_counts(FILE *stream_in, counts_t *counts, options_t *options);

#endif
