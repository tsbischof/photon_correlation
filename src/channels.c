#include "channels.h"

#include "error.h"
#include "modes.h"
#include "t2.h"
#include "t3.h"

int channels_dispatch(FILE *in_stream, FILE *out_stream, options_t *options) {
	debug("Channels!\n");

	if ( options->mode == MODE_T2 ) {
		return(channels_t2(in_stream, out_stream, options));
	} else if ( options->mode == MODE_T3 ) {
		return(channels_t3(in_stream, out_stream, options));
	} else {
		error("Mode not supported: %s\n", options->mode_string);
		return(-1);
	}
}

int channels_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	t2_t record;

	while ( ! next_t2(in_stream, &record) ) {
		if ( options->suppress_channels && 
			options->suppressed_channels[record.channel]) {
			;
		} else {
			print_t2(out_stream, &record, options);
		}
	}

	return(0);
}

int channels_t3(FILE *in_stream, FILE *out_stream, options_t *options) {
	t3_t record;
	
	while ( ! next_t3(in_stream, &record) ) {
		if ( options->suppress_channels && 
			options->suppressed_channels[record.channel]) {
			;
		} else {
			print_t3(out_stream, &record, options);
		}
	}

	return(0);
}
