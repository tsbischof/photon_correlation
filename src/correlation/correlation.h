#ifndef CORRELATION_H_
#define CORRELATION_H_

#include <stdio.h>
#include "correlation.h"

typedef struct {
	int mode;
	size_t photon_size;
	unsigned int order;
	void *photons;
} correlation_t;

typedef void (*correlate_t)(correlation_t *correlation);
typedef int (*correlation_next_t)(FILE *stream_out, 
		correlation_t *correlation);
typedef int (*correlation_print_t)(FILE *stream_out, 
		correlation_t const *correlation);

correlation_t *correlation_alloc(int const mode, unsigned int const order);
void correlation_init(correlation_t *correlation);
int correlation_set_index(correlation_t *correlation,
		unsigned int const index, void const *photon);
void correlation_free(correlation_t **correlation);

#endif

