#ifndef BIN_INTENSITY_VOID_H_
#define BIN_INTENSITY_VOID_H_

#include <stdio.h>
#include "options.h"
#include "histogram_gn.h"
#include "bin_intensity.h"
#include "photon.h"

typedef struct {
	int bins;
	int channels;
	edges_t *edges;
	size_t photon_size;
	void *front;
	void *back;
	uint64_t **bin_count;
} bin_counts_photon_t;

typedef int (*bin_counts_photon_next_t)
		(FILE *stream_in, bin_counts_photon_t *);
typedef int (*bin_counts_photon_print_t)
		(FILE *stream_out, bin_counts_photon_t *);
int bin_counts_photon_fread(FILE *stream_in, bin_counts_photon_t *);
int bin_counts_photon_fscanf(FILE *stream_in, bin_counts_photon_t *);
int bin_counts_photon_fwrite(FILE *stream_out, bin_counts_photon_t *);
int bin_counts_photon_fprintf(FILE *stream_out, bin_counts_photon_t *);

int bin_counts_echo(FILE *stream_in, FILE *stream_out, options_t *options);

int bin_intensity_photon(FILE *stream_in, FILE *stream_out, options_t *options);

bin_counts_photon_t *bin_counts_photon_alloc(options_t *options);
void bin_counts_photon_init(bin_counts_photon_t *bin_counts);
void bin_counts_photon_reset(bin_counts_photon_t *bin_counts);
void bin_counts_photon_free(bin_counts_photon_t **bin_counts);
void bin_counts_photon_increment(bin_counts_photon_t *bin_counts, 
		void const *photon);

#endif
