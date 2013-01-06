#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intensity.h"
#include "intensity_void.h"
#include "intensity_t2.h"
#include "intensity_t3.h"
#include "modes.h"
#include "error.h"

int intensity_dispatch(FILE *stream_in, FILE *stream_out, options_t *options) {
	int result;

	if ( options->use_void ) {
		result = intensity_void(stream_in, stream_out, options);
	} else if ( options->mode == MODE_T2 ) {
		result = intensity_t2(stream_in, stream_out, options);
	} else if ( options->mode == MODE_T3 ) {
		result = intensity_t3(stream_in, stream_out, options);
	} else {
		result = -1;
	}

	return(result);
}

counts_t *counts_alloc(int channels) {
	counts_t *counts;
	int result = 0;
	
	counts = (counts_t *)malloc(sizeof(counts_t));
	if ( counts == NULL ) {
		result = -1;
	} else {
		counts->channels = channels;
		counts->window.lower = 0;
		counts->window.upper = 0;
		counts->counts = (uint64_t *)malloc(sizeof(uint64_t)*channels);
		if ( counts->counts == NULL ) {
			counts_free(&counts);
			return(counts);
		}
	}

	return(counts);
}

void counts_init(counts_t *counts) {
	int i;

	for ( i = 0; i < counts->channels; i++ ) {
		counts->counts[i] = 0;
	}
}

void counts_free(counts_t **counts) {
	if ( *counts != NULL ) {
		free((*counts)->counts);
		free(*counts);
	}
}

int counts_increment(counts_t *counts, int channel) {
	if ( channel >= 0 && channel < counts->channels ) {
		counts->counts[channel]++;
		return(PC_SUCCESS);
	} else {
		error("Invalid channel %d requested for increment.\n", channel);
		return(PC_ERROR_CHANNEL);
	}
}

int counts_fread(FILE *stream_in, counts_t *counts) {
	size_t n_read;
	n_read = fread(&(counts->window), sizeof(window_t), 1, stream_in);
	if ( n_read != 1 ) {
		return(PC_ERROR_IO);
	} 

	n_read = fread(counts->counts, 
			sizeof(uint64_t), 
			counts->channels, 
			stream_in);

	if ( n_read == counts->channels ) {
		return(PC_SUCCESS);
	} else {
		if ( feof(stream_in) ) {
			return(EOF);
		} else {
			return(PC_ERROR_IO);
		}
	}
}

int counts_fscanf(FILE *stream_in, counts_t *counts) {
	int i;
	size_t n_read;

	n_read = fscanf(stream_in, 
				"%"SCNd64",%"SCNd64, 
				&(counts->window.lower),
				&(counts->window.upper));

	if ( n_read != 2) {
		return( feof(stream_in) ? PC_SUCCESS : PC_ERROR_IO );
	}

	for ( i = 0; i < counts->channels; i++ ) {
		n_read = fscanf(stream_in, 
				",%"SCNu64, 
				&(counts->counts[i]));

		if ( n_read != 1 ) {
			return( feof(stream_in) ? PC_SUCCESS : PC_ERROR_IO );
		}
	}

	return(PC_SUCCESS);
}

int counts_fwrite(FILE *stream_out, counts_t const *counts) {
	size_t n_write;

	n_write = fwrite(&(counts->window), 
			sizeof(window_t), 
			1, 
			stream_out);

	if ( n_write != 1 ) {
		return(PC_ERROR_IO);
	} 

	n_write = fwrite(counts->counts, 
			sizeof(uint64_t),
			counts->channels,
			stream_out);

	return( n_write == counts->channels ? PC_SUCCESS : PC_ERROR_IO );
}

int counts_fprintf(FILE *stream_out, counts_t const *counts) {
	int i;
	size_t n_write;

	n_write = fprintf(stream_out, 
			"%"PRId64",%"PRId64,
			counts->window.lower,
			counts->window.upper);

	if ( ferror(stream_out) ) {
		return(PC_ERROR_IO);
	}

	for ( i = 0; i < counts->channels; i++ ) {
		n_write = fprintf(stream_out,
				",%"PRIu64,
				counts->counts[i]);
	
		if ( ferror(stream_out) ) {
			return(PC_ERROR_IO);
		}
	}

	fprintf(stream_out, "\n");

	return(PC_SUCCESS);
}

int counts_echo(FILE *stream_in,  FILE *stream_out, 
		int binary_in, int binary_out, int channels) {
	counts_t *counts = counts_alloc(channels);
	counts_next_t next = COUNTS_NEXT(binary_in);
	counts_print_t print = COUNTS_PRINT(binary_out);

	if ( counts == NULL ) {
		return(PC_ERROR_MEM);
	} 

	while ( next(stream_in, counts) == PC_SUCCESS ) {
		print(stream_out, counts);
	}

	return(PC_SUCCESS);
}

/* Legacy code */
counts_t *allocate_counts(int channels) {
	counts_t *counts;
	int result = 0;
	
	counts = (counts_t *)malloc(sizeof(counts_t));
	if ( counts == NULL ) {
		result = -1;
	} else {
		counts->channels = channels;
		counts->window.lower = 0;
		counts->window.upper = 0;
		counts->counts = (uint64_t *)malloc(
				sizeof(uint64_t)*channels);
		if ( counts->counts == NULL ) {
			result = -1;
		}
	}

	if ( result ) {
		free_counts(&counts);
		counts = NULL;
	}

	return(counts);
}

void init_counts(counts_t *counts) {
	int i;

	for ( i = 0; i < counts->channels; i++ ) {
		counts->counts[i] = 0;
	}
}

void free_counts(counts_t **counts) {
	if ( *counts != NULL ) {
		free((*counts)->counts);
		free(*counts);
	}
}

int increment_counts(counts_t *counts, int channel) {
	if ( channel >= 0 && channel < counts->channels ) {
		counts->counts[channel] += 1;
		return(0);
	} else {
		error("Invalid channel %d requested for increment.\n", channel);
		return(-1);
	}
}

void print_counts(FILE *stream_out, counts_t *counts, options_t *options) {
	int i;

	if ( options->binary_out ) {
		fwrite(&(counts->window), 1, sizeof(window_t), stream_out);
		fwrite(counts->counts, options->channels, sizeof(int64_t), stream_out);
	} else {
		fprintf(stream_out, "%"PRId64",%"PRId64",", 
				counts->window.lower,
				counts->window.upper);
		for ( i = 0; i < counts->channels; i++ ) {
			fprintf(stream_out, "%"PRIu64, counts->counts[i]);
			if ( i != counts->channels - 1 ) {
				fprintf(stream_out, ",");
			}
		}
		fprintf(stream_out, "\n");
	}
}

int next_counts(FILE *stream_in, counts_t *counts, options_t *options) {
	int i;
	int result = 0;

	if ( options->binary_in ) {
		result = (fread(&(counts->window), sizeof(window_t), 
				1, stream_in) != 1);
		if ( ! result ) {
			result = (fread(counts->counts, sizeof(int64_t), 
					options->channels, stream_in) != options->channels);
		} 
	} else {
		result = (fscanf(stream_in, "%"SCNd64",%"SCNd64, 
				&(counts->window.lower),
				&(counts->window.upper)) != 2);
		for ( i = 0; ! result && i < options->channels; i++) {
			result = (fscanf(stream_in, ",%"SCNu64, &(counts->counts[i])) 
					!= 1);
		}
	}

	return(result);		
}
