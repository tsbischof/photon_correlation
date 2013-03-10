#include <string.h>

#include "gn.h"
#include "error.h"
#include "photon.h"
#include "histogram_photon.h"
#include "correlate_photon.h"

int gn(char *filename_in, pc_options_t const *options) {
/* This program must perform:
 * 1. correlation/histogramming
 * 2. intensity (normal or binned)
 */
	int result = PC_SUCCESS;
	char *histogram_filename = malloc(sizeof(char)*(strlen(filename_in) + 120));
	photon_stream_t *photon_stream = photon_stream_alloc(options);
	correlator_t *correlator = correlator_alloc(options);
	histogram_gn_t *histogram = histogram_gn_alloc(options);

	FILE *photon_file;
	FILE *histogram_file;

	debug("Initializing.\n");

	if ( histogram_filename == NULL || photon_stream == NULL || 
			correlator == NULL || histogram == NULL ) {
		error("Could not allocate space.\n");
		result = PC_ERROR_OPTIONS;
	}

	if ( result == PC_SUCCESS ) {
		photon_file = fopen(filename_in, "r");
		if ( photon_file == NULL ) {
			error("Could not open photon file.\n");
			result = PC_ERROR_OPTIONS;
		}
	}

	if ( result == PC_SUCCESS ) {
		if ( photon_stream_init(photon_stream, photon_file, options) 
				!= PC_SUCCESS ) {
			error("Could not initialize photon stream.\n");
			result = PC_ERROR_OPTIONS;
		}

		if ( correlator_init(correlator, photon_stream, options) 
				!= PC_SUCCESS ) {
			error("Could not initialize correlator.\n");
			result = PC_ERROR_OPTIONS;
		}
	}

	debug("Current result: %d\n", result);

	while ( result == PC_SUCCESS && ! photon_stream_eof(photon_stream) ) {
		debug("Starting gn.\n");
		if ( options->bin_width == 0 ) {
			sprintf(histogram_filename, "%s.g%d", 
					filename_in, options->order);
		} else {
			sprintf(histogram_filename, "%s.g%d.%020"PRId64"_%020"PRId64,
					filename_in, options->order, 
					photon_stream->window.limits.lower,
					photon_stream->window.limits.upper);
		}

		debug("Writing to  %s.\n", histogram_filename);
		if ( options->print_every != 0 ) {
			fprintf(stderr, "Writing to %s\n", histogram_filename);
		}
		histogram_file = fopen(histogram_filename, "w");

		if ( histogram_file == NULL ) {
			error("Could not open %s for writing.\n", histogram_filename);
			result = PC_ERROR_IO;
			break;
		}

		correlator_flush(correlator);
		histogram_gn_init(histogram);

		while ( correlator_next(correlator) == PC_SUCCESS ) {
			histogram_gn_increment(histogram, correlator->current_correlation);
		}

		histogram->print(histogram_file, histogram);
		fclose(histogram_file);

		photon_stream_next_window(photon_stream);
	}

	fclose(photon_file);

	free(histogram_filename);
	photon_stream_free(&photon_stream);
	correlator_free(&correlator);
	histogram_gn_free(&histogram);
	
	return(PC_SUCCESS);
}
