#include "correlate.h"
#include "correlation/correlator_log.h"
#include "correlation/photon.h"
#include "correlation/start_stop.h"
#include "limits.h"

int correlate_dispatch(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	if ( options->start_stop ) {
		return(correlate_start_stop(stream_in, stream_out, options));
	} else if ( options->time_scale == SCALE_LOG ) {
		return(correlate_log(stream_in, stream_out, options));
	} else {
		return(correlate_photon(stream_in, stream_out, options));
	}
}
