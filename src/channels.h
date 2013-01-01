#ifndef CHANNELS_H_
#define CHANNELS_H_

#include <stdio.h>

#include "options.h"
#include "channels_t2.h"
#include "channels_t3.h"

int channels_dispatch(FILE *stream_in, FILE *stream_out, options_t *options);
int64_t offset_difference(int64_t *offsets, int n);

#endif
