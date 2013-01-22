#ifndef T3_H_
#define T3_H_

#include <stdio.h>
#include "options.h"
#include "photon.h"
#include "types.h"


typedef struct {
	uint32_t channel;
	uint32_t time;
	int64_t pulse;
} t3_t;

typedef int (*t3_next_t)(FILE *, t3_t *);
typedef int (*t3_print_t)(FILE *, t3_t const *);

int t3_fread(FILE *stream_in, t3_t *t3);
int t3_fscanf(FILE *stream_in, t3_t *t3);
int t3_fprintf(FILE *stream_out, t3_t const *t3);
int t3_fwrite(FILE *stream_out, t3_t const *t3);
#define T3_NEXT(x) ( x ? t3_fread : t3_fscanf );
#define T3_PRINT(x) ( x ? t3_fwrite : t3_fprintf );

int t3_compare(void const *a, void const *b);
int t3_echo(FILE *stream_in, FILE *stream_out, int binary_in, int binary_out);

#endif
