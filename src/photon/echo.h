#ifndef PHOTON_ECHO_H_
#define PHOTON_ECHO_H_

#include <stdio.h>
#include "../options.h"

int photon_echo(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options);

#endif
