#ifndef CHANNELS_PHOTON_H_
#define CHANNELS_PHOTON_H_

#include <stdio.h>

#include "options.h"

int channels_photon(FILE *stream_in, FILE *stream_out, options_t *options);
void yield_sorted_photon(FILE *stream_out, photon_queue_t *queue, 
		int64_t max_offset_difference, options_t *options);

#endif
