#include <stdio.h>
#include <stdlib.h>

#include "t2.h"

int next_t2(FILE *in_stream, t2_t *record) {
	int result;

	result = fscanf(in_stream, "%u,%lld",
			&(record->channel),
			&(record->time));
	
	return(result != 2);
}

void print_t2(FILE *out_stream, t2_t *record) {
	fprintf(out_stream, "%u,%lld", 
			record->channel,
			record->time);
}

t2_queue_t *allocate_t2_queue(int queue_length) {
	int result = 0;
	t2_queue_t *queue;

	queue = (t2_queue_t *)malloc(sizeof(t2_queue_t));
	if ( queue == NULL ) {
		result = -1;
	} else {
		queue->length = queue_length;
		queue->left_index = -1;
		queue->right_index = -1;
	
		queue->queue = (t2_t *)malloc(sizeof(t2_t)*queue->length);
		if ( queue->queue == NULL ) {
			result = -1;
		}
	}

	if ( result ) {
		free_t2_queue(&queue);
	}

	return(queue);
}

void free_t2_queue(t2_queue_t **queue) {
	if ( *queue != NULL ) {
		if ( (*queue)->queue != NULL ) {
			free((*queue)->queue);
		}
		free(*queue);
	}
}

t2_t get_queue_item_t2(t2_queue_t *queue, int index) {
	int true_index = (queue->left_index + index) % queue->length;
	return(queue->queue[true_index]);
}

