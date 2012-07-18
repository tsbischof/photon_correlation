#include "histogram.h"
#include "histogram_t2.h"
#include "histogram_t3.h"
#include "error.h"
#include "modes.h"

int histogram_dispatch(FILE *in_stream, FILE *out_stream, options_t *options) {
	int result;

	if ( options->mode == MODE_T2 ) {
		debug("Mode t2.\n");
		result = histogram_t2(in_stream, out_stream, options);
	} else if ( options->mode == MODE_T3 ) {
		debug("Mode t3.\n");
		result =  histogram_t3(in_stream, out_stream, options);
	} else {
		result = -1;
	}

	return(result);
}
