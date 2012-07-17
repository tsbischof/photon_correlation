#ifndef T3_H_
#define T3_H_

#include <stdio.h>

typedef struct {
	unsigned int channel;
	long long int pulse_number;
	int time;
} t3_t;


typedef struct {
	int length;
	long long int left_index;
	long long int right_index;
	t3_t *queue;
} t3_queue_t;

int next_t3(FILE *in_stream, t3_t *record);
void print_t3(FILE *out_stream, t3_t *record);


t3_queue_t *allocate_t3_queue(int queue_length);
void free_t3_queue(t3_queue_t **queue);
t3_t get_queue_item_t3(t3_queue_t *queue, int index);

#endif
