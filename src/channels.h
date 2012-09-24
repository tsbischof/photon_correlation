#ifndef CHANNELS_H_
#define CHANNELS_H_

#include <stdio.h>

#include "options.h"
#include "channels_t2.h"
#include "channels_t3.h"

typedef struct {
	int first_photon;
	int last_photon;
	int n_photons;
	photon_t;
	temp_photon;
	*photons;
	photon_comparator;
	photon_distance;
	max_photon_distance;
} sorted_photons_t; 

int channels_dispatch(FILE *in_stream, FILE *out_stream, options_t *options);

sorted_photons_t *allocate_sorted_photons();
void free_sorted_photons(sorted_photons_t *photons);

int sorted_photon_insert(sorted_photons_t *photons, photon, options_t *options);
int yield_sorted_photon(sorted_photons_t *photons, photon, int eof, 
		options_t *options);
/* At EOF, yield photons until the stream is exhausted. */

#endif
