#include "correlate.h"
#include "correlate_photon.h"

int correlate_dispatch(FILE *stream_in, FILE *stream_out, 
		options_t const *options) {
	return(correlate_photon(stream_in, stream_out, options));
}
