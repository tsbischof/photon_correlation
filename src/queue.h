/*
 * Copyright (c) 2011-2014, Thomas Bischof
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Massachusetts Institute of Technology nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

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

int queue_index_copy(queue_t const *queue, void *elem, size_t const index);
int queue_index(queue_t const *queue, void **elem, size_t const index);
int queue_pop(queue_t *queue, void *elem);
int queue_push(queue_t *queue, void const *elem);
int queue_front(queue_t const *queue, void **elem);
int queue_front_copy(queue_t const *queue, void *elem);
int queue_back(queue_t const *queue, void **elem);
int queue_back_copy(queue_t const *queue, void *elem);

#endif
