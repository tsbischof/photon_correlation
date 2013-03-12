#include <stdlib.h>
#include <string.h>

#include "correlation.h"
#include "../modes.h"
#include "../photon/t2.h"
#include "../photon/t3.h"
#include "../error.h"

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

int correlation_set_index(correlation_t *correlation,
		unsigned int const index, void const *photon) {
	if ( index >= correlation->order ) {
		error("Index too large for correlation: %u (limit %u)\n", 
				index,
				correlation->order);
		return(PC_ERROR_INDEX);
	} 

	memcpy(&(correlation->photons[index*correlation->photon_size]),
			photon,
			correlation->photon_size);

	return(PC_SUCCESS);
}

void correlation_free(correlation_t **correlation) {
	if ( *correlation != NULL ) {
		free((*correlation)->photons);
		free(*correlation);
	}
}

