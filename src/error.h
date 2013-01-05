#ifndef ERROR_H_
#define ERROR_H_

#define PC_SUCCESS                    0
#define PC_ERROR_IO                  -1
#define PC_ERROR_UNKNOWN             -2
#define PC_ERROR_OPTIONS             -3
#define PC_ERROR_QUEUE_OVERFLOW      -4
#define PC_ERROR_QUEUE_INDEX         -5
#define PC_ERROR_QUEUE_EMPTY         -6
#define PC_ERROR_MEM                 -7
#define PC_ERROR_WINDOW_AHEAD        -8
#define PC_ERROR_MODE                -9
#define PC_ERROR_CHANNEL            -10
#define PC_ERROR_INDEX              -11

#define PC_USAGE                      1
#define PC_WINDOW_NEXT                2
#define PC_WINDOW_EXCEEDED            3

#define BYTES_CHECK(x,y) (x == y ? PC_SUCCESS : PC_ERROR_IO)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "options.h"

extern int verbose;

void debug(char const *message, ...);
void error(char const *message, ...);
void warn(char const *message, ...);

int pc_status_print(char const *name, 
		long long record_number, options_t *options);
int pc_check(int error_code);

#endif
