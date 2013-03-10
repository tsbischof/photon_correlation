#include <stdlib.h>
#include <string.h>

#include "counts.h"
#include "../error.h"

counts_t *counts_alloc(unsigned int const bins) {
	counts_t *counts = NULL;

	counts = (counts_t *)malloc(sizeof(counts_t));

	if ( counts == NULL ) {
		return(counts);
	}

	counts->bins = bins;
	counts->counts = (unsigned long long *)malloc(
			sizeof(unsigned long long)*counts->bins);

	if ( counts->counts == NULL ) {
		counts_free(&counts);
		return(counts);
	}

	return(counts);
}

void counts_init(counts_t *counts) {
	memset(counts->counts, 0, counts->bins*sizeof(unsigned long long));
}

int counts_increment(counts_t *counts, unsigned int const index) {
	if ( index < counts->bins ) {
		counts->counts[index]++;
		return(PC_SUCCESS);
	} else {
		return(PC_ERROR_INDEX);
	}
}

int counts_increment_number(counts_t *counts, unsigned int const index, 
		unsigned long long const number) {
	if ( index < counts->bins ) {
		counts->counts[index] += number;
		return(PC_SUCCESS);
	} else {
		return(PC_ERROR_INDEX);
	}
}

void counts_free(counts_t **counts) {
	if ( *counts != NULL ) {
		free((*counts)->counts);
		free(*counts);
	}
}

int counts_nonzero(counts_t const *counts) {
	int i;

	for ( i = 0; i < counts->bins; i++ ) {
		if ( counts->counts[i] != 0 ) {
			return(true);
		}
	}

	return(false);
}
