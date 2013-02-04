#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "files.h"
#include "error.h"
#include "t2.h"
#include "t2_void.h"
#include "photon.h"

/* 
 * Functions to implement t2 photon read/write.
 */
int t2_fread(FILE *stream_in, t2_t *t2) {
	size_t n_read = fread(t2, sizeof(t2_t), 1, stream_in);

	if ( n_read == 1 ) {
		return(PC_SUCCESS);
	} else {
		return( feof(stream_in) ? EOF : PC_ERROR_IO );
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
		return( feof(stream_in) ? EOF : PC_ERROR_IO );
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

void t2_correlate(correlation_t *correlation) {
	int i;

	for ( i = 1; i < correlation->order; i++ ) {
		((t2_t *)(correlation->photons))[i].time -=
				((t2_t *)(correlation->photons))[0].time;
	}

	((t2_t *)(correlation->photons))[0].time = 0;
}

int t2_correlation_fread(FILE *stream_in, correlation_t *correlation) {
	size_t n_read;

	n_read = fread(correlation->photons,
			sizeof(t2_t),
			correlation->order,
			stream_in);

	if ( n_read == correlation->order ) {
		return(PC_SUCCESS);
	} else {
		return( feof(stream_in) ? EOF : PC_ERROR_IO );
	}
}

int t2_correlation_fscanf(FILE *stream_in, correlation_t *correlation) {
	int i;
	int result;

	result = fscanf(stream_in,
			"%"PRIu32,
			&(((t2_t *)(correlation->photons))[0].channel));

	result = (result == 1);

	((t2_t *)(correlation->photons))[0].time = 0;

	for ( i = 1; result && i < correlation->order; i++ ) {
		result = fscanf(stream_in,
				",%"PRIu32",%"PRId64,
				&(((t2_t *)(correlation->photons))[i].channel),
				&(((t2_t *)(correlation->photons))[i].time));

		result = (result == 2);
	}

	fscanf(stream_in, "\n");

	if ( ! result ) {
		return( feof(stream_in) ? EOF : PC_ERROR_IO );
	} else {
		return(PC_SUCCESS);
	}
}

int t2_correlation_fprintf(FILE *stream_out, correlation_t const *correlation) {
	int i;

	if ( correlation->order == 1 ) {
		t2v_fprintf(stream_out, correlation->photons);
	} else {
		fprintf(stream_out, 
				"%"PRIu32, 
				((t2_t *)(correlation->photons))[0].channel);

		for ( i = 1; i < correlation->order; i++ ) {
			fprintf(stream_out, 
					",%"PRIu32",%"PRId64,
					((t2_t *)(correlation->photons))[i].channel,
					((t2_t *)(correlation->photons))[i].time);
		}

		fprintf(stream_out, "\n");
	}

	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
}

int t2_correlation_fwrite(FILE *stream_out, correlation_t const *correlation) {
	fwrite(correlation->photons,
			sizeof(t2_t),
			correlation->order,
			stream_out);

	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
}

int t2_under_max_distance(void *correlator) {
	int64_t max = ((correlator_t *)correlator)->max_time_distance;
	t2_t *left = ((correlator_t *)correlator)->left;
	t2_t *right = ((correlator_t *)correlator)->right;

	return( max == 0 || i64abs(right->time - left->time) < max );
}

int t2_over_min_distance(void *correlator) {
	int64_t min = ((correlator_t *)correlator)->min_time_distance;
	t2_t *left = (t2_t *)((correlator_t *)correlator)->left;
	t2_t *right = (t2_t *)((correlator_t *)correlator)->right;

	return( i64abs(right->time - left->time) >= min );
}

