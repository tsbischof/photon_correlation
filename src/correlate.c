#include "correlate.h"
#include "correlate_t2.h"
#include "correlate_t3.h"
#include "error.h"
#include "files.h"
#include "modes.h"

int correlate_dispatch(FILE *in_stream, FILE *out_stream, options_t *options) {
	int result;

	debug("Checking the mode.\n");
	if ( options->mode == MODE_T2 ) {
		debug("Mode t2.\n");
		result = correlate_t2(in_stream, out_stream, options);
	} else if ( options->mode == MODE_T3 ) {
		debug("Mode t3.\n");
		result = correlate_t3(in_stream, out_stream, options);
	}  else {
		result = -1;
	}

	return(result);
}
