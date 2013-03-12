#ifndef T2_H_
#define T2_H_

#include <stdio.h>

typedef struct {
	unsigned int channel;
	long long time;
} t2_t;

typedef int (*t2_next_t)(FILE *, t2_t *);
typedef int (*t2v_next_t)(FILE *, void *);
typedef int (*t2_print_t)(FILE *, t2_t const *);
typedef int (*t2v_print_t)(FILE *, void const *);

int t2_fscanf(FILE *stream_in, t2_t *t2);
int t2v_fscanf(FILE *stream_in, void *t2);
int t2_fprintf(FILE *stream_out, t2_t const *t2);
int t2v_fprintf(FILE *stream_out, void const *t2);

int t2v_compare(void const *a, void const *b);
int t2_echo(FILE *stream_in, FILE *stream_out);

long long t2v_window_dimension(void const *t2);
long long t2v_channel_dimension(void const *t2);

#endif
