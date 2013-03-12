#ifndef INDEX_OFFSETS_H_
#define INDEX_OFFSETS_H_

#include "../options.h"
#include "combinations.h"

typedef struct {
	unsigned int length;
	unsigned int limit;
	int yielded;

	combination_t *current_index_offsets;
} index_offsets_t;

index_offsets_t *index_offsets_alloc(unsigned int const length);
void index_offsets_init(index_offsets_t *index_offsets, 
		unsigned int const limit);
int index_offsets_next(index_offsets_t *index_offsets);
void index_offsets_free(index_offsets_t **index_offsets);

#endif 
