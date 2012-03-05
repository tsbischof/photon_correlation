#include <stdio.h>

#include "histogram.h"
#include "histogram_t3.h"
#include "histogram_t3_g1.h"
#include "t3.h"
#include "error.h"

int histogram_t3(FILE *in_stream, FILE *out_stream, options_t *options) {
	if ( options->order == 1 ) {
		debug("Doing first-order correlation.\n");
		histogram_t3_g1(in_stream, out_stream, options);
	} else {
		error("High-order t3 correlations not yet implemented.\n");
		return(-1);
	}

	return(0);
}
