#ifndef NUMBER_TO_CHANNELS_H_
#define NUMBER_TO_CHANNELS_H_

#include <stdio.h>
#include "../queue.h"
#include "../photon/t3.h"
#include "../options.h"

typedef struct {
	int flushing;

	long long current_pulse;
	queue_t *queue;
	t3_t photon;

	unsigned int current_channel;
	unsigned int seen_this_pulse;
} number_to_channels_t;

number_to_channels_t *number_to_channels_alloc(size_t const queue_size);
void number_to_channels_init(number_to_channels_t *number);
int number_to_channels_push(number_to_channels_t *number, t3_t const *t3);
int number_to_channels_next(number_to_channels_t *number);
void number_to_channels_flush(number_to_channels_t *number);
void number_to_channels_pulse_over(number_to_channels_t *number);
void number_to_channels_free(number_to_channels_t **number);

int number_to_channels(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
