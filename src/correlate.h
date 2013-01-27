#ifndef CORRELATE_H_
#define CORRELATE_H_

#include <stdio.h>
#include "options.h"

int correlate_dispatch(FILE *stream_in, FILE *stream_out, 
		options_t const *options);

#endif
