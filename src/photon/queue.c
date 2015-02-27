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

#include "queue.h"
#include "t2.h"
#include "t3.h"
#include "../modes.h"

queue_t *photon_queue_alloc(int const mode, size_t const length) {
	photon_queue_t *queue = NULL;

	queue = queue_alloc(sizeof(photon_t), length);

	if ( queue == NULL ) {
		return(queue);
	}

	if ( mode == MODE_T2 ) {
		queue_set_comparator(queue, t2_compare);
	} else if ( mode == MODE_T3 ) {
		queue_set_comparator(queue, t3_compare);
	} else {
		photon_queue_free(&queue);
	} 

	return(queue);
}

void photon_queue_init(photon_queue_t *queue) {
	queue_init(queue);
}

void photon_queue_free(photon_queue_t **queue) {
	queue_free(queue);
}

int photon_queue_full(photon_queue_t const *queue) {
	return(queue_full(queue));
}

int photon_queue_empty(photon_queue_t const *queue) {
	return(queue_empty(queue));
}

size_t photon_queue_size(photon_queue_t const *queue) {
	return(queue_size(queue));
}

size_t photon_queue_capacity(photon_queue_t const *queue) {
	return(queue_capacity(queue));
}

int photon_queue_resize(photon_queue_t *queue, size_t const length) {
	return(queue_resize(queue, length));
}

int photon_queue_sort(photon_queue_t *queue) {
	return(queue_sort(queue));
}

int photon_queue_index_copy(photon_queue_t const *queue, 
		photon_t *photon, size_t const index) {
	return(queue_index_copy(queue, (void *)photon, index));	
}
int photon_queue_index(photon_queue_t const *queue, 
		photon_t **photon, size_t const index) {
	return(queue_index(queue, (void **)photon, index));
}

int photon_queue_pop(photon_queue_t *queue, photon_t *photon) {
	return(queue_pop(queue, (void *)photon));
}

int photon_queue_push(photon_queue_t *queue, photon_t const *photon) {
	return(queue_push(queue, (void *)photon));
}

int photon_queue_front(photon_queue_t const *queue, photon_t **photon) {
	return(queue_front(queue, (void **)photon));
}

int photon_queue_front_copy(photon_queue_t const *queue, photon_t *photon) {
	return(queue_front_copy(queue, (void *)photon));
}

int photon_queue_back(photon_queue_t const *queue, photon_t **photon) {
	return(queue_back(queue, (void **)photon));
}

int photon_queue_back_copy(photon_queue_t const *queue, photon_t *photon) {
	return(queue_back_copy(queue, (void *)photon));
}
