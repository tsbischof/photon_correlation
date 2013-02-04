#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "files.h"
#include "error.h"
#include "t3.h"
#include "t3_void.h"
#include "photon.h"

/* 
 * Functions to implement t3 photon read/write.
 */
int t3_fread(FILE *stream_in, t3_t *t3) {
	size_t n_read = fread(t3, sizeof(t3_t), 1, stream_in);

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

int t3_fscanf(FILE *stream_in, t3_t *t3) {
	int n_read = fscanf(stream_in,
			"%"SCNu32",%"SCNd64",%"SCNd64"\n",
			&(t3->channel),
			&(t3->pulse),
			&(t3->time));

	if ( n_read == 3 ) {
		return(PC_SUCCESS);
	} else {
		if ( feof(stream_in) ) { 
			return(EOF);
		} else {
			return(PC_ERROR_IO);
		}
	} 
}

int t3_fprintf(FILE *stream_out, t3_t const *t3) {
	fprintf(stream_out,
			"%"PRIu32",%"PRId64",%"PRId64"\n",
			t3->channel,
			t3->pulse,
			t3->time);

	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
}

int t3_fwrite(FILE *stream_out, t3_t const *t3) {
	size_t n_write = fwrite(t3,
			sizeof(t3_t),
			1,
			stream_out);

	return( n_write == 1 ? PC_SUCCESS : PC_ERROR_IO );
}

int t3_compare(void const *a, void const *b) {
	/* Comparator to be used with standard sorting algorithms (qsort) to sort
	 * t3 photons. 
     */
	/* The comparison must be done explicitly to avoid issues associated with
	 * casting int64_t to int. If we just return the difference, any value
	 * greater than max_int would cause problems.
	 */
	/* The comparator needed for sorting a list of t3 photons. Follows the 
     * standard of qsort (-1 sorted, 0 equal, 1 unsorted)
	 */
	int64_t difference;

	if ( ((t3_t *)a)->pulse == ((t3_t *)b)->pulse ) {
		difference = ((t3_t *)a)->time - ((t3_t *)b)->time;
	} else {
		difference = ((t3_t *)a)->pulse - ((t3_t *)b)->pulse;
	}

	return( difference > 0 );
}

int t3_echo(FILE *stream_in, FILE *stream_out, int binary_in, int binary_out) {
	t3_t t3;
	t3_next_t next = T3_NEXT(binary_in);
	t3_print_t print = T3_PRINT(binary_out);

	while ( next(stream_in, &t3) == PC_SUCCESS ) {
		print(stream_out, &t3);
	}

	return(PC_SUCCESS);
}


void t3_correlate(correlation_t *correlation) {
	int i;

	for ( i = 1; i < correlation->order; i++ ) {
		((t3_t *)(correlation->photons))[i].pulse -=
				((t3_t *)(correlation->photons))[0].pulse;
		((t3_t *)(correlation->photons))[i].time -=
				((t3_t *)(correlation->photons))[0].time; 
	}

	if ( correlation->order != 1 ) {
		((t3_t *)(correlation->photons))[0].pulse = 0;
		((t3_t *)(correlation->photons))[0].time = 0;
	}
}

int t3_correlation_fread(FILE *stream_in, correlation_t *correlation) {
	size_t n_read;

	n_read = fread(correlation->photons,
			sizeof(t3_t),
			correlation->order,
			stream_in);

	if ( n_read == correlation->order ) {
		return(PC_SUCCESS);
	} else {
		return( feof(stream_in) ? EOF : PC_ERROR_IO );
	}
}

int t3_correlation_fscanf(FILE *stream_in, correlation_t *correlation) {
	int i;
	int result;

	result = fscanf(stream_in,
			"%"PRIu32,
			&(((t3_t *)(correlation->photons))[0].channel));

	result = (result == 1);

	((t3_t *)(correlation->photons))[0].pulse = 0;
	((t3_t *)(correlation->photons))[0].time = 0;

	for ( i = 1; result && i < correlation->order; i++ ) {
		result = fscanf(stream_in,
				",%"PRIu32",%"PRId64",%"PRId64,
				&(((t3_t *)(correlation->photons))[i].channel),
				&(((t3_t *)(correlation->photons))[i].pulse),
				&(((t3_t *)(correlation->photons))[i].time));

		result = (result == 3);
	}

	fscanf(stream_in, "\n");

	if ( ! result ) {
		return( feof(stream_in) ? EOF : PC_ERROR_IO );
	} else {
		return(PC_SUCCESS);
	}
}

int t3_correlation_fprintf(FILE *stream_out, correlation_t const *correlation) {
	int i;

	if ( correlation->order == 1 ) {
		t3v_fprintf(stream_out, correlation->photons);
	} else {
		fprintf(stream_out, 
				"%"PRIu32, 
				((t3_t *)(correlation->photons))[0].channel);

		for ( i = 1; i < correlation->order; i++ ) {
			fprintf(stream_out, 
					",%"PRIu32",%"PRId64",%"PRId64,
					((t3_t *)correlation->photons)[i].channel,
					((t3_t *)correlation->photons)[i].pulse,
					((t3_t *)correlation->photons)[i].time);
		}

		fprintf(stream_out, "\n");
	}

	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
}

int t3_correlation_fwrite(FILE *stream_out, correlation_t const *correlation) {
	fwrite(correlation->photons,
			sizeof(t3_t),
			correlation->order,
			stream_out);
	
	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
}

int t3_under_max_distance(void const *correlator) {
	int64_t max_time = ((correlator_t *)correlator)->max_time_distance;
	int64_t max_pulse = ((correlator_t *)correlator)->max_pulse_distance;
	t3_t *left = ((correlator_t *)correlator)->left;
	t3_t *right = ((correlator_t *)correlator)->right;

	return( (max_time == 0 
				|| i64abs(right->time - left->time) < max_time)
			&& (max_pulse == 0 
				|| i64abs(right->pulse - left->pulse) < max_pulse) );
}

int t3_over_min_distance(void const *correlator) {
	int64_t min_time = ((correlator_t *)correlator)->min_time_distance;
	int64_t min_pulse = ((correlator_t *)correlator)->min_pulse_distance;
	t3_t *left = (t3_t *)((correlator_t *)correlator)->left;
	t3_t *right = (t3_t *)((correlator_t *)correlator)->right;

	return( (min_time == 0 || 
				i64abs(right->time - left->time) >= min_time)
			&& (min_pulse == 0 || 
				i64abs(right->pulse - left->pulse) >= min_pulse) );
}

