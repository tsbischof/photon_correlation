#ifndef CHANNELS_H_
#define CHANNELS_H_

#include <stdio.h>

#include "options.h"

int channels_dispatch(FILE *in_stream, FILE *out_stream, options_t *options);

#endif
