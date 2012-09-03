#include "bin_intensity.h"
#include "bin_intensity_t2.h"
#include "bin_intensity_t3.h"
#include "modes.h"
#include "error.h"

int bin_intensity_dispatch(FILE *in_stream, FILE *out_stream, 
		options_t *options) {
	int result;

	debug("Checking the mode.\n");
	if ( options->mode == MODE_T2 ) {
		debug("Mode t2.\n");
		result = bin_intensity_t2(in_stream, out_stream, options);
	} else if ( options->mode == MODE_T3 ) {
		debug("Mode t3.\n");
		error("Mode t3 not yet supported.\n");
		result = - 1;
	} else {
		result = -1;
	}

	return(result);
}
