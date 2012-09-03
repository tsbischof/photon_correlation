#include "channels.h"
#include "channels_t2.h"

#include "error.h"
#include "t2.h"

int channels_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	t2_t record;

	while ( ! next_t2(in_stream, &record) ) {
		if ( options->suppress_channels && 
			options->suppressed_channels[record.channel]) {
			;
		} else {
			offset_t2(&record, options);

			while ( ! yield_sorted_photon(photons, &record, !EOF, options) ) {
				print_t2(out_stream, &record, options);
			}
		}
	}

	while ( ! yield_sorted_photon(photons, &record, EOF, options) ) {
		print_t2(out_stream, record, options);
	}

	return(0);
}

void offset_t2(t2_t *record, options_t *options) {
	if ( options->offset_channels ) {
		record->time += options->channel_offsets[record->channel];
	}	
}

