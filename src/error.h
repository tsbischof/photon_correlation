/*
 * Copyright (c) 2011-2015, Thomas Bischof
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Massachusetts Institute of Technology nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

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
#define PC_ERROR_OVERFLOW                  -16

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
