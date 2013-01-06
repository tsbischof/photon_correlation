#include <stdio.h>
#include <stdlib.h>

#include "intensity_to_t2.h"
#include "intensity.h"
#include "t2.h"
#include "options.h"
#include "error.h"

int intensity_fread(FILE *stream_in, intensity_t *intensity) {
	size_t n_read = fread(intensity,
			sizeof(intensity_t),
			1,
			stream_in);

	if ( n_read == 1 ) {
		return(PC_SUCCESS);
	} else {
		return( feof(stream_in) ? EOF : PC_ERROR_IO );
	}
}

int intensity_fscanf(FILE *stream_in, intensity_t *intensity) { 
	int n_read = fscanf(stream_in, "%"PRId64",%"PRIu32"\n",
			&(intensity->time),
			&(intensity->counts));

	if ( n_read == 2 ) {
		return(PC_SUCCESS);
	} else {
		if ( feof(stream_in) ) {
			return(EOF);
		} else {
			return(PC_ERROR_IO);
		}
	}
}

int intensity_to_t2(FILE *stream_in, FILE *stream_out, options_t *options) {
	int result = 0;
	int i;
	intensity_t intensity;
	intensity_next_t intensity_next = INTENSITY_NEXT(options->binary_in);
	t2_print_t t2_print = T2_PRINT(options->binary_out);
	t2_t t2;

	srand(options->seed);

	while ( (result = intensity_next(stream_in, &intensity)) == PC_SUCCESS ) {
		for ( i = 0; i < intensity.counts; i++ ) {
			t2.channel = rand() % options->channels;
			t2.time = intensity.time;

			t2_print(stream_out, &t2);
		}
	}

	if ( result == EOF ) {
		result = PC_SUCCESS;
	}

	return(result);
}
