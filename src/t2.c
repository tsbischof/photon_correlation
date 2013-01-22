#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "files.h"
#include "error.h"
#include "t2.h"
#include "photon.h"

/* 
 * Functions to implement t2 photon read/write.
 */
int t2_fread(FILE *stream_in, t2_t *t2) {
	size_t n_read = fread(t2, sizeof(t2_t), 1, stream_in);

	if ( n_read == 1 ) {
		return(PC_SUCCESS);
	} else {
		if ( feof(stream_in) ) {
			return(EOF);
		} else {
			return(PC_ERROR_IO);
		}
	}	
}

int t2_fscanf(FILE *stream_in, t2_t *t2) {
	int n_read = fscanf(stream_in,
			"%"SCNu32",%"SCNd64,
			&(t2->channel),
			&(t2->time));

	if ( n_read == 2 ) {
		return(PC_SUCCESS);
	} else {
		if ( feof(stream_in) ) { 
			return(EOF);
		} else {
			return(PC_ERROR_IO);
		}
	} 
}

int t2_fprintf(FILE *stream_out, t2_t const *t2) {
	fprintf(stream_out,
			"%"PRIu32",%"PRId64"\n",
			t2->channel,
			t2->time);

	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
}

int t2_fwrite(FILE *stream_out, t2_t const *t2) {
	size_t n_write = fwrite(t2,
			sizeof(t2_t),
			1,
			stream_out);

	return( n_write == 1 ? PC_SUCCESS : PC_ERROR_IO );
}

int t2_compare(void const *a, void const *b) {
	/* Comparator to be used with standard sorting algorithms (qsort) to sort
	 * t2 photons. 
	 * The comparison must be done explicitly to avoid issues associated with
	 * casting int64_t to int. If we just return the difference, any value
	 * greater than max_int would cause problems.
	 */
	int64_t difference = ((t2_t *)a)->time - ((t2_t *)b)->time;
	return( difference > 0 );
}

int t2_echo(FILE *stream_in, FILE *stream_out, int binary_in, int binary_out) {
	t2_t t2;
	t2_next_t next = T2_NEXT(binary_in);
	t2_print_t print = T2_PRINT(binary_out);

	while ( next(stream_in, &t2) == PC_SUCCESS ) {
		print(stream_out, &t2);
	}

	return(PC_SUCCESS);
}

