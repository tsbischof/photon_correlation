#ifndef T2_VOID_H_
#define T2_VOID_H_

#include "t2.h"
#include "channels.h"

typedef int (*t2v_next_t)(FILE *, void *);
typedef int (*t2v_print_t)(FILE *, void const *);

int t2v_fscanf(FILE *stream_in, void *photon);
int t2v_fprintf(FILE *stream_out, void const *photon);

int64_t t2v_window_dimension(void const *photon);
int64_t t2v_channel_dimension(void const *photon);

void t2v_offset(void *photon, offsets_t const *offsets);

#endif
