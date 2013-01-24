#include <stdlib.h>
#include <string.h>

#include "correlate.h"
#include "error.h"
#include "modes.h"
#include "t2.h"
#include "t3.h"

int correlate_dispatch(FILE *stream_in, FILE *stream_out, 
		options_t const *options) {
	return(correlate_photon(stream_in, stream_out, options));
}

correlation_t *correlation_alloc(int const mode, unsigned int const order) {
	correlation_t *correlation = NULL;

	correlation = (correlation_t *)malloc(sizeof(correlation_t));
	if ( correlation == NULL ) {
		return(correlation);
	}

	correlation->mode = mode;
	correlation->order = order;

	if ( mode == MODE_T2 ) {
		correlation->photon_size = sizeof(t2_t);
	} else if ( mode == MODE_T3 ) {
		correlation->photon_size = sizeof(t3_t);
	} else {
		error("Unsupported mode: %d\n", mode);
		correlation_free(&correlation);
	}

	correlation->photons = malloc(correlation->photon_size*order);

	if ( correlation->photons == NULL ) {
		correlation_free(&correlation);
		return(correlation);
	}

	return(correlation);
}

void correlation_init(correlation_t *correlation) {
	memset(correlation->photons, 
			0, 
			correlation->photon_size*correlation->order);
}

void correlation_free(correlation_t **correlation) {
	if ( *correlation != NULL ) {
		free((*correlation)->photons);
		free(*correlation);
	}
}

correlator_t *correlator_alloc(options_t const *options) {
	correlator_t *correlator = NULL;

	return(correlator);
}

int correlator_init(correlator_t *correlator, photon_stream_t *photon_stream,
		options_t const *options) {
	return(EOF);
}

int correlator_next(correlator_t *correlator) {
	return(EOF);
}

void correlator_free(correlator_t **correlator) {

}

int correlate_photon(FILE *stream_in, FILE *stream_out, 
		options_t const *options) {
	photon_stream_t *photon_stream = photon_stream_alloc(options);
	correlator_t *correlator = correlator_alloc(options);

	debug("Allocating correlator, photon stream.\n");
	if ( correlator == NULL || photon_stream == NULL ) {
		error("Could not allocate correlator (%p), photon stream (%p).\n",
				correlator, photon_stream);
		correlator_free(&correlator);
		photon_stream_free(&photon_stream);
		return(PC_ERROR_MEM);
	}

	debug("Initializing correlator, photon stream.\n");
	photon_stream_init(photon_stream, stream_in, options);
	correlator_init(correlator, photon_stream, options);

	debug("Streaming correlations.\n");
	while ( correlator_next(correlator) == PC_SUCCESS ) {
		correlator->correlation_print(stream_out, 
				correlator->current_correlation);
	}

	debug("Freeing correlator, photon stream.\n");
	correlator_free(&correlator);
	photon_stream_free(&photon_stream);

	return(PC_SUCCESS);
}

