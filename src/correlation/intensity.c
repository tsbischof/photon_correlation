#include <math.h>

#include "intensity.h"
#include "../statistics/intensity.h"
#include "../modes.h"
#include "../error.h"
#include "multi_tau.h"

int intensity_correlate_g2_log(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	multi_tau_g2cn_t *mt;
	intensity_photon_t *intensity;

	mt = multi_tau_g2cn_alloc(options->binning, options->registers, 
			options->depth, options->channels, 1);
	intensity = intensity_photon_alloc(options->channels, MODE_T2);

	if ( mt == NULL || intensity == NULL ) {
		error("Could not allocate correlator or intensity stream.\n");
		result = PC_ERROR_MEM;
	}

	if ( result == PC_SUCCESS ) {
		intensity_photon_init_stream(intensity, stream_in);
		multi_tau_g2cn_init(mt);

		while ( intensity_photon_next(intensity) == PC_SUCCESS ) {
			multi_tau_g2cn_push(mt, intensity->counts);
		}
	}

	if ( result == PC_SUCCESS ) {
		multi_tau_g2cn_fprintf(stream_out, mt);
	}

	multi_tau_g2cn_free(&mt);
	intensity_photon_free(&intensity);

	return(PC_SUCCESS);
}

int intensity_correlate_dispatch(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	if ( options->order == 2 && options->time_scale == SCALE_LOG ) {
		return(intensity_correlate_g2_log(stream_in, stream_out, options));
	} else {
		error("Correlation mode not supported: order %u and scale %d\n",
				options->order, options->time_scale);
		return(PC_ERROR_OPTIONS);
	}
}
