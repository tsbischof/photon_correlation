#ifndef VALUES_VECTOR_H_
#define VALUES_VECTOR_H_

#include "edges.h"

typedef struct {
	size_t length;
	long long *values;
} values_vector_t;

values_vector_t *values_vector_alloc(unsigned int const length);
void values_vector_init(values_vector_t *vv);
void values_vector_free(values_vector_t **vv);
long long values_vector_index(values_vector_t const *vv, 
		edges_t ** const edges);
#endif
