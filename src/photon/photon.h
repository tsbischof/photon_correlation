#ifndef PHOTON_H_
#define PHOTON_H_

#include <stdio.h>
#include "../options.h"

typedef int (*photon_next_t)(FILE *, void *);
typedef int (*photon_print_t)(FILE *, void const *);

typedef long long (*photon_window_dimension_t)(void const *);
typedef long long (*photon_channel_dimension_t)(void const *);

int photon_echo(FILE *stream_in, FILE *stream_out, pc_options_t *options);

#endif
