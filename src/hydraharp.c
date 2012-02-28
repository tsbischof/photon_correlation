#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "hydraharp.h"
#include "hydraharp/hh_v10.h"

#include "picoquant.h"
#include "error.h"

int hh_dispatch(FILE *in_stream, FILE *out_stream, pq_header_t *pq_header, 
		pq_options_t *options) {
	int result;

	if ( ! strcmp("1.0", pq_header->FormatVersion) ) {
		result = hh_v10_dispatch(in_stream, out_stream, pq_header, options);
	} else {
		error("HydraHarp version not supported: %s\n",
				pq_header->FormatVersion);
	}

	return(result);
}

