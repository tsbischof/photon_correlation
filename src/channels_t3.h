#ifndef CHANNELS_T3_H_
#define CHANNELS_T3_H_

#include <stdio.h>

#include "options.h"
#include "t3.h"

int channels_t3(FILE *in_stream, FILE *out_stream, options_t *options);
void offset_t3(t3_t *record, options_t *options);

#endif
