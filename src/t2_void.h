#ifndef T2_VOID_H_
#define T2_VOID_H_

#include "t2.h"

typedef int (*t2v_next_t)(FILE *, void *);
typedef int (*t2v_print_t)(FILE *, void const *);

int t2v_fread(FILE *stream_in, void *photon);
int t2v_fscanf(FILE *stream_in, void *photon);
int t2v_fprintf(FILE *stream_out, void const *photon);
int t2v_fwrite(FILE *stream_out, void const *photon);

int64_t t2v_window_dimension(void const *photon);
int64_t t2v_channel_dimension(void const *photon);
int t2v_echo(FILE *stream_in, FILE *stream_out, int binary_in, int binary_out);

#define T2V_NEXT(x) (x ? t2v_fread : t2v_fscanf);
#define T2V_PRINT(x) (x ? t2v_fwrite : t2v_fprintf);

#endif
