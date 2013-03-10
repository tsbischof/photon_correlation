#ifndef OFFSETS_H_
#define OFFSETS_H_

typedef struct {
	unsigned int channels;

	int offset_time;
	long long *time_offsets;
	int offset_pulse;
	long long *pulse_offsets;
} offsets_t;

typedef void (*photon_offset_t)(void *photon, offsets_t const *offsets);

offsets_t *offsets_alloc(unsigned int const channels);
void offsets_init(offsets_t *offsets, 
		int const offset_time, long long const *time_offsets,
		int const offset_pulse, long long const *pulse_offsets);
void offsets_free(offsets_t **offsets);
long long offset_span(long long const *offsets, unsigned int const channels);

void t2v_offset(void *photon, offsets_t const *offsets);
void t3v_offset(void *photon, offsets_t const *offsets);

#endif
