#include <stdlib.h>

#include "bin_intensity_photon.h"
#include "photon.h"
#include "error.h"
#include "intensity.h"

int bin_counts_photon_fread(FILE *stream_in, bin_counts_photon_t *bin_counts) {
	fwindow_t window;
	int i;

	for ( i = 0; i < bin_counts->edges->n_bins; i++ ) {
		if ( fread(&window, sizeof(fwindow_t), 1, stream_in) != 1 ) { 
			break;
		}

		if ( fread(bin_counts->bin_count[i], 
				sizeof(uint64_t), 
				bin_counts->channels, 
				stream_in) != bin_counts->channels ) { 
			break;
		}

		if ( window.lower != bin_counts->edges->bin_edges[i] ||
				window.upper != bin_counts->edges->bin_edges[i+1] ) {
			error("Edges read from file do not match those "
					"already in memory.\n");
			return(PC_ERROR_MISMATCH);
		}
	}

	return(PC_SUCCESS);
}

int bin_counts_photon_fscanf(FILE *stream_in, bin_counts_photon_t *bin_counts) {
	return(-1);
}

int bin_counts_photon_fwrite(FILE *stream_out, 
			bin_counts_photon_t *bin_counts) {
	return(-1);
}

int bin_counts_photon_fprintf(FILE *stream_out, 
			bin_counts_photon_t *bin_counts) {
	return(-1);
}

int bin_counts_echo(FILE *stream_in, FILE *stream_out, options_t *options) {
	return(-1);
}

bin_counts_photon_t *bin_counts_photon_alloc(options_t *options) {
	bin_counts_photon_t *bin_counts = NULL;
	return(bin_counts);
}

void bin_counts_photon_init(bin_counts_photon_t *bin_counts) {
}

void bin_counts_photon_reset(bin_counts_photon_t *bin_counts) {
}

void bin_counts_photon_free(bin_counts_photon_t **bin_counts) {
}

void bin_counts_photon_increment(bin_counts_photon_t *bin_counts,
		void const *photon) {
}

int bin_intensity_photon(FILE *stream_in, FILE *stream_out, 
		options_t *options) {
	return(PC_SUCCESS);
}
