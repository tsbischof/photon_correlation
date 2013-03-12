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
} queue_t;

queue_t *queue_alloc(size_t const elem_size, size_t const length);
void queue_init(queue_t *queue);
void queue_free(queue_t **queue);

int queue_full(queue_t const *queue);
int queue_empty(queue_t const *queue);
size_t queue_size(queue_t const *queue);
size_t queue_capacity(queue_t const *queue);
int queue_resize(queue_t *queue, size_t const length);

void queue_set_comparator(queue_t *queue, compare_t compare);
int queue_sort(queue_t *queue);

int queue_index(queue_t const *queue, void *elem, size_t const index);
int queue_pop(queue_t *queue, void *elem);
int queue_push(queue_t *queue, void const *elem);
int queue_front(queue_t const *queue, void *elem);
int queue_back(queue_t const *queue, void *elem);

#endif
