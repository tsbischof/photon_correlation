#include "echo.h"
#include "stream.h"

int photon_echo(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	photon_stream_t *photon_stream = photon_stream_alloc(options);

	if ( photon_stream == NULL ) {
		result = PC_ERROR_MEM;
	}

	if ( result == PC_SUCCESS ) {
		result = photon_stream_init(photon_stream, stream_in, options);
	}

	if ( result == PC_SUCCESS ) {
		while ( photon_stream_next_photon(photon_stream) == PC_SUCCESS) {
			photon_stream->photon_print(stream_out, 
					photon_stream->current_photon);
		}
	}

	photon_stream_free(&photon_stream);

	return(result);
}
