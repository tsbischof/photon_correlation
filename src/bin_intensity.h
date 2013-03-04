#ifndef BIN_INTENSITY_H_
#define BIN_INTENSITY_H_

#include <stdio.h>
#include "options.h"

int bin_intensity_dispatch(FILE *stream_in, FILE *stream_out, 
		options_t const *options);

#endif
