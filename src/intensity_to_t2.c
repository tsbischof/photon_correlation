#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "intensity_to_t2.h"
#include "t2.h"
#include "options.h"

int next_intensity(FILE *stream_in, intensity_t *intensity, 
		options_t *options) {
	int result;

	if ( options->binary_in ) {
		result = ( fread(intensity, sizeof(intensity_t), 1, 
				stream_in) != 1);
	} else {
		result = ( fscanf(stream_in, "%"PRId64",%"PRIu32, 
				&(intensity->time), &(intensity->counts)) != 2);
	} 

	return(result);
}

int intensity_to_t2(FILE *stream_in, FILE *stream_out, options_t *options) {
	int result = 0;
	int i;
	intensity_t intensity;
	t2_t record;

	srand(time(NULL));

	while ( ! result && ! next_intensity(stream_in, &intensity, options) ) {
		for ( i = 0; i < intensity.counts; i++ ) {
			record.channel = rand() % options->channels;
			record.time = intensity.time;

			print_t2(stream_out, &record, NEWLINE, options);
		}
	}

	return(result);
}
