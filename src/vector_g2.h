#ifndef VECTOR_G2_H_
#define VECTOR_G2_H_

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "options.h"
#include "limits.h"
#include "histogram_gn.h"

typedef struct {
	int64_t sum;
	int64_t number;
} fraction_t;

typedef struct {
	char name[100];
	limits_t limits;
	edges_t *edges;
	int *delays;
	fraction_t *g2;
} vector_g2_t;

void correlate_vector_g2(fraction_t *signal_0, fraction_t *signal_1, int length,
		vector_g2_t *g2);
void print_vector_g2(FILE *out_stream, vector_g2_t *g2, options_t *options);
vector_g2_t *allocate_vector_g2(limits_t *limits, int scale);
void free_vector_g2(vector_g2_t **g2);

#endif
