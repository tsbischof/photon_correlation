#ifndef T2_H_
#define T2_H_

#include <stdio.h>

#include "types.h"
#include "options.h"
#include "correlate_photon.h"

typedef struct {
	uint32_t channel;
	int64_t time;
} t2_t;

typedef int (*t2_next_t)(FILE *, t2_t *);
typedef int (*t2_print_t)(FILE *, t2_t const *);

int t2_fread(FILE *stream_in, t2_t *t2);
int t2_fscanf(FILE *stream_in, t2_t *t2);
int t2_fprintf(FILE *stream_out, t2_t const *t2);
int t2_fwrite(FILE *stream_out, t2_t const *t2);
#define T2_NEXT(x) ( x ? t2_fread : t2_fscanf );
#define T2_PRINT(x) ( x ? t2_fwrite : t2_fprintf );

int t2_compare(void const *a, void const *b);
int t2_echo(FILE *stream_in, FILE *stream_out, int binary_in, int binary_out);

void t2_correlate(correlation_t *correlation);
int t2_correlation_fread(FILE *stream_in, correlation_t *correlation);
int t2_correlation_fscanf(FILE *stream_in, correlation_t *correlation);
int t2_correlation_fprintf(FILE *stream_out, correlation_t const *correlation);
int t2_correlation_fwrite(FILE *stream_out, correlation_t const *correlation);
#define T2_CORRELATION_NEXT(x) ( x ? t2_correlation_fread : t2_correlation_fscanf );
#define T2_CORRELATION_PRINT(x) ( x ? t2_correlation_fwrite : t2_correlation_fprintf );

int t2_under_max_distance(void const *correlator);
int t2_over_min_distance(void const *correlator);


#endif
