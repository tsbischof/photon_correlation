#include "histogram.h"
#include "histogram_photon.h"

int histogram_dispatch(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	return(histogram_photon(stream_in, stream_out, options));
}
