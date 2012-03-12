#include <stdio.h>
#include <stdarg.h>

#include "error.h"

int verbose = 0;

void debug(char *message, ...) {
	/* If in debug mode, we want to print a message. Otherwise, we want
	 * to ignore it.
	 */
	va_list args;
	va_start(args, message);
	if ( verbose ) {
		fprintf(stderr, "DEBUG: ");
		vfprintf(stderr, message, args);
	}
	va_end(args);
	fflush(stderr);
}

void error(char *message, ...) {
	/* Handle error messages. This is here in case we ever want to do 
	 * something more than just print them.
	 */
	va_list args;
	va_start(args, message);
	fprintf(stderr, "ERROR: ");
	vfprintf(stderr, message, args);
	va_end(args);
	fflush(stderr);
}

void warn(char *message, ...) {
	/* Warnings about untested features, etc. */
	va_list args;
	va_start(args, message);
	fprintf(stderr, "WARNING: ");
	vfprintf(stderr, message, args);
	va_end(args);
	fflush(stderr);
}
