#include <stdio.h>
#include <stdlib.h>

#include "t3.h"

int next_t3(FILE *in_stream, t3_t *record) {
	int result;

	result = fscanf(in_stream, "%u,%lld,%u",
			&(record->channel),
			&(record->pulse_number),
			&(record->time));
	
	return(result != 3);
}

void print_t3(FILE *out_stream, t3_t *record) {
	fprintf(out_stream, "%u,%lld,%u", 
			record->channel,
			record->pulse_number,
			record->time);
}


t3_queue_t *allocate_t3_queue(int queue_length) {
	int result = 0;
	t3_queue_t *queue;

	queue = (t3_queue_t *)malloc(sizeof(t3_queue_t));
	if ( queue == NULL ) {
		result = -1;
	} else {
		queue->length = queue_length;
		queue->left_index = -1;
		queue->right_index = -1;
	
		queue->queue = (t3_t *)malloc(sizeof(t3_t)*queue->length);
		if ( queue->queue == NULL ) {
			result = -1;
		}
	}

	if ( result ) {
		free_t3_queue(&queue);
	}

	return(queue);
}

void free_t3_queue(t3_queue_t **queue) {
	if ( *queue != NULL ) {
		if ( (*queue)->queue != NULL ) {
			free((*queue)->queue);
		}
		free(*queue);
	}
}

t3_t get_queue_item_t3(t3_queue_t *queue, int index) {
	int true_index = (queue->left_index + index) % queue->length;
	return(queue->queue[true_index]);
}

