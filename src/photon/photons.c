#include "stream.h"
#include "t2.h"
#include "t3.h"
#include "conversions.h"
#include "../modes.h"
#include "../error.h"

int photons_echo(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) {
	int result = PC_SUCCESS;
	photon_stream_t *photon_stream = photon_stream_alloc(options->mode);

	if ( photon_stream == NULL ) {
		result = PC_ERROR_MEM;
	}

	if ( result == PC_SUCCESS ) {
		photon_stream_init(photon_stream, stream_in);

		while ( photon_stream_next_photon(photon_stream) == PC_SUCCESS) {
			photon_stream->photon_print(stream_out, 
					photon_stream->photon);
		}
	}

	photon_stream_free(&photon_stream);

	return(result);
}
 
int photons(FILE *stream_in, FILE *stream_out,
		pc_options_t const *options) {
	t2_t t2;
	t3_t t3;
	int result = PC_SUCCESS;
	photon_stream_t *photons;

	photons = photon_stream_alloc(options->mode);

	if ( photons == NULL ) {
		result = PC_ERROR_MEM;
	} 

	if ( result == PC_SUCCESS ) {
		if ( (options->convert == MODE_T2 || options->convert == MODE_T3 ) &&
				options->mode != options->convert &&
				options->repetition_rate == 0 ) {
			error("Must have nonzero repetition rate.\n");
			result = PC_ERROR_ZERO_DIVISION;
		}
	}

	if ( result == PC_SUCCESS ) {
		photon_stream_init(photons, stream_in);
		photon_stream_set_unwindowed(photons);
		photon_stream_set_windowed(photons,
				10,
				false, 0,
				false, 0);

		if ( options->convert == options->mode ||
				options->convert == MODE_UNKNOWN ) {
			debug("Echo photons.\n");
			while ( ! photon_stream_eof(photons) ) {
				while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
					photons->photon_print(stream_out, photons->photon);
				}
			
				photon_stream_next_window(photons);
			}
		} else if ( options->mode == MODE_T2 && options->convert == MODE_T3 ) {
			debug("t2 to t3\n");
			while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
				t2_to_t3(photons->photon, &t3, options->repetition_rate);
				t3_fprintf(stream_out, &t3);
			}
		} else if ( options->mode == MODE_T3 && options->convert == MODE_T2 ) {
			debug("t3 to t2\n");
			while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
				t3_to_t2(photons->photon, &t2, options->repetition_rate);
				t2_fprintf(stream_out, &t2);
			}
		} else if ( options->mode == MODE_T3 && 
				options->convert == MODE_AS_T2 ) {
			debug("t3 as t2\n");
			while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
				t3_as_t2(photons->photon, &t2);
				t2_fprintf(stream_out, &t2);
			}
		} else {
			error("Invalid photon conversion: %d to %d\n", 
					options->mode, options->convert);
			result = PC_ERROR_MODE;
		}
	}
		
	photon_stream_free(&photons);

	return(PC_SUCCESS);
}
