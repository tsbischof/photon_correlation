#ifndef PHOTONS_H_
#define PHOTONS_H_

#include <stdio.h>
#include "../options.h"

int photons_echo(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);
int photons(FILE *stream_in, FILE *stream_out,
		pc_options_t const *options);

#endif
