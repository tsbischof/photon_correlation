#ifndef VECTOR_H_
#define VECTOR_H_

#include "types.h"

typedef struct {
	size_t length;
	size_t elem_size;
	int empty;

	size_t left_index;
	size_t right_index;

	void *values;

	compare_t compare;
} vector_t;

vector_t *vector_alloc(size_t const elem_size, size_t const length);
void vector_init(vector_t *vector);
void vector_free(vector_t **vector);

int vector_full(vector_t const *vector);
int vector_empty(vector_t const *vector);
size_t vector_size(vector_t const *vector);
size_t vector_capacity(vector_t const *vector);
int vector_resize(vector_t *vector, size_t const length);

void vector_set_comparator(vector_t *vector, compare_t compare);
int vector_sort(vector_t *vector);

int vector_index(vector_t const *vector, void *elem, size_t const index);
int vector_pop(vector_t *vector, void *elem);
int vector_push(vector_t *vector, void const *elem);
int vector_front(vector_t const *vector, void *elem);
int vector_back(vector_t const *vector, void *elem);

#endif
