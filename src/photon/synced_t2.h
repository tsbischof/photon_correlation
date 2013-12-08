#ifndef SYNCED_T2_H_
#define SYNCED_T2_H_

#include <stdio.h>
#include "../options.h"
#include "t2.h"
#include "t3.h"
#include "../queue.h"

typedef struct {
	unsigned long long sync_index;

	unsigned int sync_channel;
	unsigned int sync_divider;

	t2_t last_sync;
	t2_t next_sync;

	t3_t photon;

	int flushing;
	int first_sync_seen;

	queue_t *queue;
} synced_t2_t;

int synced_t2_dispatch(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);
int synced_t2(FILE *stream_in, FILE *stream_out, pc_options_t const *options);

synced_t2_t *synced_t2_alloc(size_t const queue_size);
void synced_t2_free(synced_t2_t **synced_t2);
void synced_t2_init(synced_t2_t *synced_t2, 
		unsigned int const sync_channel, unsigned int const sync_divider);
int synced_t2_push(synced_t2_t *synced_t2, t2_t const *t2);
void synced_t2_flush(synced_t2_t *synced_t2);
int synced_t2_next(synced_t2_t *synced_t2);

#endif
