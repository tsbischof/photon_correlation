#include "channels.h"
#include "channels_t3.h"

#include "error.h"

int channels_t3(FILE *in_stream, FILE *out_stream, options_t *options) {
	t3_t record;

	while ( ! next_t3(in_stream, &record, options) ) {
		if ( options->suppress_channels && 
			options->suppressed_channels[record.channel]) {
			;
		} else {
			offset_t3(&record, options);

/*			while ( ! yield_sorted_photon(photons, &record, !EOF, options) ) {
				print_t3(out_stream, &record, options);
			} */
		}
	}

/*	while ( ! yield_sorted_photon(photons, &record, EOF, options) ) {
		print_t3(out_stream, record, options);
	} */

	return(0);
}

void offset_t3(t3_t *record, options_t *options) {
	if ( options->offset_pulse ) {
		record->pulse_number += options->pulse_offsets[record->channel];
	}

	if ( options->offset_time ) {
		record->time += options->time_offsets[record->channel];
	}	
}

