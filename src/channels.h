#ifndef CHANNELS_H_
#define CHANNELS_H_

#include <stdio.h>

#include "options.h"
#include "types.h"

typedef struct {
	int channels;
	int offset_time;
	int64_t *time_offsets;
	int offset_pulse;
	int64_t *pulse_offsets;
} offsets_t;

int channels_dispatch(FILE *stream_in, FILE *stream_out, 
		options_t const *options);

typedef void (*photon_offset_t)(void *photon, offsets_t const *offsets);
void t3v_offset(void *photon, offsets_t const *offsets);
void t2v_offset(void *photon, offsets_t const *offsets);

offsets_t *offsets_alloc(int const channels);
void offsets_init(offsets_t *offsets, options_t const *options);
void offsets_free(offsets_t **offsets);
int64_t offset_difference(int64_t const *offsets, int const channels);


#endif
