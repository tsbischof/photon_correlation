#ifndef INTENSITY_CORRELATE_H_
#define INTENSITY_CORRELATE_H_

#include <stdio.h>
#include "../options.h"

int intensity_correlate_g2_log(FILE *stream_in, FILE *stream_out,
		pc_options_t const *options);
int intensity_correlate_dispatch(FILE *stream_in, FILE *stream_out,
		pc_options_t const *options);

#endif
