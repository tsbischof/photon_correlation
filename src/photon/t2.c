#include "t2.h"

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

int t2v_fscanf(FILE *stream_in, void *t2) {
	return(t2_fscanf(stream_in, t2));
}

int t2_fprintf(FILE *stream_out, t2_t const *t2) {
	fprintf(stream_out,
			"%u,%lld\n",
			t2->channel,
			t2->time);

	return( ! ferror(stream_out) ? PC_SUCCESS : PC_ERROR_IO );
}

int t2v_fprintf(FILE *stream_out, void const *t2) {
	return(t2_fprintf(stream_out, t2));
}

int t2v_compare(void const *a, void const *b) {
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

long long t2v_window_dimension(void const *t2) {
	return(((t2_t *)t2)->time);
}

long long t2v_channel_dimension(void const *t2) {
	return(((t2_t *)t2)->channel);
}
