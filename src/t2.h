#ifndef T2_H_
#define T2_H_

#include <stdio.h>

#include "types.h"
#include "photon.h"
#include "options.h"

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

#endif
