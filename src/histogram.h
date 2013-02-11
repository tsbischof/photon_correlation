#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include <stdio.h>
#include "options.h"

int histogram_dispatch(FILE *stream_in, FILE *stream_out, 
		options_t const *options);

#endif
