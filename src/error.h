#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern int verbose;

void debug(char *message, ...);
void error(char *message, ...);
void warn(char *message, ...);

#endif
