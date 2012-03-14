#ifndef T2_H_
#define T2_H_

#include <stdio.h>

typedef struct {
	unsigned int channel;
	long long int time;
} t2_t;

int next_t2(FILE *in_stream, t2_t *record);
void print_t2(FILE *out_stream, t2_t *record);


#endif
