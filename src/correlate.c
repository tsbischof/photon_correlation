#include "correlate.h"
#include "correlate_photon.h"
#include "correlate_start_stop.h"

int correlate_dispatch(FILE *stream_in, FILE *stream_out, 
		options_t const *options) {
	if ( options->start_stop ) {
		return(correlate_start_stop(stream_in, stream_out, options));
	} else {
		return(correlate_photon(stream_in, stream_out, options));
	}
}
