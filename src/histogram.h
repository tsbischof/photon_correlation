#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include <stdio.h>
#include "options.h"

int histogram_dispatch(FILE *stream_in, FILE *stream_out, options_t *options);

typedef struct {

} histogram_t;

histogram_t *histogram_alloc(options_t *options);
int histogram_init(histogram_t *histogram);
void histogram_reset(histogram_t *histogram);
void histogram_free(histogram_t **histogram);
int histogram_increment(histogram_t *histogram, correlation_t *correlation);

typedef int (*histogram_print_t)(FILE *, histogram_t *);
int histogram_fprintf(FILE *stream_out, histogram_t *histogram);
int histogram_fwrite(FILE *stream_out, histogram_t *histogram);

#endif
