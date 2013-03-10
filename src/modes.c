#include <stdio.h>
#include <string.h>

#include "error.h"
#include "modes.h"

int mode_parse(int *mode, char const *mode_string) { 
	if ( mode_string == NULL ) {
		*mode = MODE_UNKNOWN;
		return(PC_ERROR_MODE);
	} 

	if ( !strcmp("t2", mode_string) ) {
		debug("Found mode t2.\n");
		*mode = MODE_T2;
		return(PC_SUCCESS);
	} else if ( !strcmp("t3", mode_string) ) {
		debug("Found mode t3.\n");
		*mode = MODE_T3;
		return(PC_SUCCESS);
	} else if ( ! strcmp("vector", mode_string) ) {
		debug("Found mode vector.\n");
		*mode = MODE_VECTOR;
		return(PC_SUCCESS);
	} else if ( ! strcmp("as-t2", mode_string) ) {
		debug("Found mode as-t2.\n");
		*mode = MODE_AS_T2;
		return(PC_SUCCESS);
	} else {
		error("Mode not recognized: %s.\n", mode_string);
		*mode = MODE_UNKNOWN;
		return(PC_ERROR_MODE);
	}
}
