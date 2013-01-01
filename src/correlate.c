#include "correlate.h"
#include "error.h"
#include "files.h"
#include "modes.h"

int correlate_dispatch(FILE *stream_in, FILE *stream_out, options_t *options) {
	int result;

	debug("Checking the mode.\n");
	if ( options->mode == MODE_T2 ) {
		debug("Mode t2.\n");
		if ( options->start_stop ) {
			result = correlate_t2_start_stop(stream_in, stream_out,
					options);
		} else {
			result = correlate_t2(stream_in, stream_out, options);
		}
	} else if ( options->mode == MODE_T3 ) {
		debug("Mode t3.\n");
		result = correlate_t3(stream_in, stream_out, options);
	}  else {
		result = -1;
	}

	return(result);
}
