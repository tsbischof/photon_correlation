#ifndef T3_H_
#define T3_H_

#include <stdio.h>

typedef struct {
	unsigned int channel;
	long long int pulse_number;
	int time;
} t3_t;

int next_t3(FILE *in_stream, t3_t *record);
void print_t3(FILE *out_stream, t3_t *record);

#endif
