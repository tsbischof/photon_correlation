#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "error.h"

int verbose = 0;

void debug(char const *message, ...) {
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

void error(char const *message, ...) {
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

void warn(char const *message, ...) {
	/* Warnings about untested features, etc. */
	va_list args;
	va_start(args, message);
	fprintf(stderr, "WARNING: ");
	vfprintf(stderr, message, args);
	va_end(args);
	fflush(stderr);
}

int pc_status_print(char const *name, uint64_t count, 
		pc_options_t const *options) {
	time_t rawtime;
	struct tm *timeinfo;
	char fmttime[80];

	if ( (options->print_every > 0) && 
			( (count % options->print_every) == 0 ) ) {
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(fmttime, 80, "%Y.%m.%d %H.%M.%S", timeinfo);
		fprintf(stderr, 
				"%s: (%s) Record %20"PRIu64"\n", 
				fmttime, 
				name, 
				count);

		if ( ferror(stderr) ) {
			return(PC_ERROR_IO);
		} else {
			return(PC_SUCCESS);
		}
	} else {
		return(PC_SUCCESS);
	}
}

int pc_check(int error_code) {
	if ( error_code < PC_SUCCESS ) {
		return(error_code);
	} else {
		return(PC_SUCCESS);
	}
}
