#ifndef CORRELATE_H_
#define CORRELATE_H_

#include <stdio.h>
#include "options.h"
#include "correlate_t2.h"
#include "correlate_t3.h"

int correlate_dispatch(FILE *in_stream, FILE *out_stream, options_t *options);

#endif

