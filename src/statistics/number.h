#ifndef NUMBER_H_
#define NUMBER_H_

#include <stdio.h>
#include "counts.h"
#include "../options.h"
#include "../photon/t3.h"

typedef struct {
	unsigned int max_number;
	unsigned int max_seen;

	int first_seen;
	long long last_pulse;
	unsigned long long current_seen;

	counts_t *counts;
} photon_number_t;

photon_number_t *photon_number_alloc(unsigned int max_number);
void photon_number_init(photon_number_t *number);
void photon_number_free(photon_number_t **number);

int photon_number_push(photon_number_t *number, t3_t const *t3);
int photon_number_increment(photon_number_t *number, 
		unsigned int const n_photons, unsigned long long seen);
int photon_number_check_max(photon_number_t *number);
int photon_number_flush(photon_number_t *number);
int photon_number_fprintf(FILE *stream_out, photon_number_t const *number);

int photon_number(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
