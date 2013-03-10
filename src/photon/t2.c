#include "t2.h"
#include "t2_void.h"

#include "../error.h"

/* 
 * Functions to implement t2 photon read/write.
 */
int t2_fscanf(FILE *stream_in, t2_t *t2) {
	int n_read = fscanf(stream_in,
			"%u,%lld",
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
			"%u,%lld\n",
			t2->channel,
			t2->time);

	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
}

int t2_compare(void const *a, void const *b) {
	/* Comparator to be used with standard sorting algorithms (qsort) to sort
	 * t2 photons. 
	 * The comparison must be done explicitly to avoid issues associated with
	 * casting long long to int. If we just return the difference, any value
	 * greater than max_int would cause problems.
	 */
	long long difference = ((t2_t *)a)->time - ((t2_t *)b)->time;
	return( difference > 0 );
}

int t2_echo(FILE *stream_in, FILE *stream_out) {
	t2_t t2;

	while ( t2_fscanf(stream_in, &t2) == PC_SUCCESS ) {
		t2_fprintf(stream_out, &t2);
	}

	return(PC_SUCCESS);
}

/*void t2_correlate(correlation_t *correlation) {
	int i;

	for ( i = 1; i < correlation->order; i++ ) {
		((t2_t *)correlation->photons)[i].time -=
				((t2_t *)correlation->photons)[0].time;
	}

	if ( correlation->order != 1 ) {
		((t2_t *)correlation->photons)[0].time = 0;
	}
}

int t2_correlation_fscanf(FILE *stream_in, correlation_t *correlation) {
	int i;
	int result;

	result = fscanf(stream_in,
			"%"PRIu32,
			&(((t2_t *)correlation->photons)[0].channel));

	result = (result == 1);

	((t2_t *)(correlation->photons))[0].time = 0;

	for ( i = 1; result && i < correlation->order; i++ ) {
		result = fscanf(stream_in,
				",%"PRIu32",%"PRId64,
				&(((t2_t *)correlation->photons)[i].channel),
				&(((t2_t *)correlation->photons)[i].time));

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
				((t2_t *)correlation->photons)[0].channel);

		for ( i = 1; i < correlation->order; i++ ) {
			fprintf(stream_out, 
					",%"PRIu32",%"PRId64,
					((t2_t *)correlation->photons)[i].channel,
					((t2_t *)correlation->photons)[i].time);
		}

		fprintf(stream_out, "\n");
	}

	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
}

int t2_under_max_distance(void const *correlator) {
	long long max = ((correlator_t *)correlator)->max_time_distance;
	t2_t *left = ((correlator_t *)correlator)->left;
	t2_t *right = ((correlator_t *)correlator)->right;

	return( max == 0 || llabs(right->time - left->time) < max );
}

int t2_over_min_distance(void const *correlator) {
	long long min = ((correlator_t *)correlator)->min_time_distance;
	t2_t *left = (t2_t *)((correlator_t *)correlator)->left;
	t2_t *right = (t2_t *)((correlator_t *)correlator)->right;

	return( min == 0 || llabs(right->time - left->time) >= min );
}

int t2_correlation_build_channels(correlation_t const *correlation,
		combination_t *channels_vector) {
	int i;

	for ( i = 0; i < correlation->order; i++ ) {
		channels_vector->values[i] = (uint32_t)
				((t2_t *)correlation->photons)[i].channel;
	}

	return(PC_SUCCESS);
}

int t2_correlation_build_values(correlation_t const *correlation,
		values_vector_t *values_vector) {
	int i;

	for ( i = 1; i < correlation->order; i++ ) {
		values_vector->values[i-1] = 
				(long long)((t2_t *)correlation->photons)[i].time;
	}

	return(PC_SUCCESS);
}
*/
