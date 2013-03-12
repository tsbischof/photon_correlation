#ifndef GN_H_
#define GN_H_

#include <stdio.h>
#include "options.h"

int gn_run(program_options_t *program_options, int const argc, 
		char * const *argv);
int gn(FILE *stream_in, FILE *stream_out, pc_options_t const *options);

#endif
