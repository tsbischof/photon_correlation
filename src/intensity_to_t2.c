#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "intensity_to_t2.h"
#include "t2.h"
#include "options.h"

int next_intensity(FILE *in_stream, intensity_t *intensity) {
	return( fscanf(in_stream, "%lld,%u",
			&(intensity->time),
			&(intensity->counts)) != 2 );
}

int intensity_to_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	int result = 0;
	int i;
	intensity_t intensity;
	t2_t record;

	srand(time(NULL));

	while ( ! result && ! next_intensity(in_stream, &intensity) ) {
		for ( i = 0; i < intensity.counts; i++ ) {
			record.channel = rand() % options->channels;
			record.time = intensity.time;

			print_t2(out_stream, &record);
			printf("\n");
		}
	}

	return(result);
}
