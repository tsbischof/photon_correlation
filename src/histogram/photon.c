#include "photon.h"
#include "histogram_gn.h"
#include "../correlation/correlation.h"
#include "../correlation/photon.h"
#include "../photon/t2.h"
#include "../photon/t3.h"
#include "../error.h"
#include "../modes.h"
#include "../options.h"

int t2_correlation_build_channels(correlation_t const *correlation,
		combination_t *channels_vector) {
	int i;

	for ( i = 0; i < correlation->order; i++ ) {
		channels_vector->values[i] = (unsigned int)
				((t2_t *)correlation->photons)[i].channel;
	}

	return(PC_SUCCESS);
}

int t2_correlation_build_values(correlation_t const *correlation,
		values_vector_t *values_vector) {
	int i;

	for ( i = 1; i < correlation->order; i++ ) {
		values_vector->values[i-1] = 
				(long long)((t2_t *)correlation->photons)[i].time;
	}

	return(PC_SUCCESS);
}

int t3_correlation_build_channels(correlation_t const *correlation,
		combination_t *channels_vector) {
	int i;

	for ( i = 0; i < correlation->order; i++ ) {
		channels_vector->values[i] = 
				(unsigned int)((t3_t *)correlation->photons)[i].channel;
	}

	return(PC_SUCCESS);
}

int t3_correlation_build_values(correlation_t const *correlation,
		values_vector_t *values_vector) {
	int i;

	if ( correlation->order == 1 ) {
		values_vector->values[0] = 
				(long long)((t3_t *)correlation->photons)[0].time;
	} else {
		for ( i = 1; i < correlation->order; i++ ) {
			values_vector->values[2*(i-1)] = 
					(long long)((t3_t *)correlation->photons)[i].pulse;
			values_vector->values[2*(i-1)+1] = 
					(long long)((t3_t *)correlation->photons)[i].time;
		}
	}

	return(PC_SUCCESS);
}


int histogram_photon(FILE *stream_in, FILE *stream_out,
		pc_options_t const *options) {
	int result = PC_ERROR_UNKNOWN;
	histogram_gn_t *hist = NULL;
	correlation_t *correlation = NULL;
	correlation_next_t next;
	correlation_print_t print;

	hist = histogram_gn_alloc(options->mode, options->order,
			options->channels, 
			options->time_scale, &(options->time_limits),
			options->pulse_scale, &(options->pulse_limits));
	correlation = correlation_alloc(options->mode, options->order);

	if ( hist == NULL || correlation == NULL ) {
		error("Could not allocate histogram or correlation.\n");
		histogram_gn_free(&hist);
		correlation_free(&correlation);
		return(PC_ERROR_MEM);
	}

	if ( options->mode == MODE_T2 ) {
		next = t2_correlation_fscanf;
		print = t2_correlation_fprintf;
	} else if ( options->mode == MODE_T3 ) {
		next = t3_correlation_fscanf;
		print = t3_correlation_fprintf;
	} else { 
		error("Invalid mode: %d\n", options->mode);
		histogram_gn_free(&hist);
		correlation_free(&correlation);
		return(PC_ERROR_MODE);
	}

	correlation_init(correlation);
	histogram_gn_init(hist);

	while ( next(stream_in, correlation) == PC_SUCCESS ) {
		if ( verbose ) {
			debug("Incrementing for correlation: \n");
			if ( correlation->mode == MODE_T2 ) {
				t2_correlation_fprintf(stderr, correlation);
			} else if ( correlation->mode == MODE_T3 ) {
				t3_correlation_fprintf(stderr, correlation);
			} 
		}
		result = histogram_gn_increment(hist, correlation);
		if ( result != PC_SUCCESS ) {
			error("Could not increment with correlation:\n");
			print(stderr, correlation);
		}
	}

	debug("Finished reading correlations from stream.\n");

	if ( result == PC_SUCCESS ) {
		hist->print(stream_out, hist);
	}

	histogram_gn_free(&hist);
	correlation_free(&correlation); 

	return(PC_SUCCESS);
}
