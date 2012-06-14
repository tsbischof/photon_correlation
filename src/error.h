#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "options.h"

extern int verbose;

void debug(char *message, ...);
void error(char *message, ...);
void warn(char *message, ...);

void print_status(long long int record_number, options_t *options);

#endif
