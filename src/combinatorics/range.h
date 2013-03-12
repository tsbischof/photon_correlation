#ifndef RANGE_H_
#define RANGE_H_

#include <stdio.h>

typedef struct {
	int yielded;
	unsigned int start;
	unsigned int stop;
	unsigned int value;
} range_t;

typedef struct {
	unsigned int dimensions;
	range_t **ranges;
} ranges_t;

range_t *range_alloc(void);
void range_init(range_t *range);
void range_set_stop(range_t *range, unsigned int const stop);
void range_set_start(range_t *range, unsigned int const start);
int range_next(range_t *range);
void range_free(range_t **range);

ranges_t *ranges_alloc(unsigned int const dimensions);
void ranges_init(ranges_t *ranges);
void ranges_free(ranges_t **ranges);
int ranges_set_start(ranges_t *ranges, unsigned int const dimension, 
		unsigned int const start);
void ranges_set_starts(ranges_t *ranges, unsigned int const start);
int ranges_set_stop(ranges_t *ranges, unsigned int const dimension, 
		unsigned int const stop);
void ranges_set_stops(ranges_t *ranges, unsigned int const stop);
int ranges_next(ranges_t *ranges);

int ranges_fprintf(FILE *stream_out, ranges_t const *ranges);

#endif
