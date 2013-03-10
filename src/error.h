#ifndef ERROR_H_
#define ERROR_H_

#define PC_SUCCESS                           0
#define PC_ERROR_IO                       -100 // EOF == -1
#define PC_ERROR_UNKNOWN                    -2
#define PC_ERROR_OPTIONS                    -3
#define PC_ERROR_QUEUE_OVERFLOW             -4
#define PC_ERROR_QUEUE_INDEX                -5
#define PC_ERROR_QUEUE_EMPTY                -6
#define PC_ERROR_MEM                        -7
#define PC_ERROR_MODE                       -9
#define PC_ERROR_CHANNEL                   -10
#define PC_ERROR_INDEX                     -11
#define PC_ERROR_MISMATCH                  -12
#define PC_ERROR_ZERO_DIVISION             -13
#define PC_ERROR_BEFORE_WINDOW             -14
#define PC_ERROR_NO_RECORD_AVAILABLE       -15

#define PC_USAGE                             1
#define PC_WINDOW_NEXT                       2
#define PC_WINDOW_EXCEEDED                   3
#define PC_WINDOW_AHEAD                      4
#define PC_COMBINATION_OVERFLOW              5
#define PC_VERSION                           6
#define PC_RECORD_AVAILABLE                 PC_SUCCESS 

#define PC_RECORD_BEFORE_WINDOW             10
#define PC_RECORD_IN_WINDOW                 PC_SUCCESS
#define PC_RECORD_AFTER_WINDOW              11

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "options.h"

extern int verbose;

void debug(char const *message, ...);
void error(char const *message, ...);
void warn(char const *message, ...);

int pc_status_print(char const *name, 
		uint64_t record_number, pc_options_t const *options);
int pc_check(int error_code);

#endif
