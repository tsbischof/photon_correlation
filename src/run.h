#ifndef RUN_H_
#define RUN_H_

#include <stdio.h>

#include "options.h"

typedef int (*dispatch_t)(FILE *, FILE *, pc_options_t const*);

int run(program_options_t *program_options, dispatch_t const dispatch,
		int const argc, char * const *argv);

#endif
