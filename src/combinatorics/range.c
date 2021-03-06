/*
 * Copyright (c) 2011-2015, Thomas Bischof
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Massachusetts Institute of Technology nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>

#include "range.h"
#include "../error.h"

range_t *range_alloc(void) {
	range_t *range = NULL;

	range = (range_t *)malloc(sizeof(range_t));

	if ( range == NULL ) {
		return(range);
	}

	range->start = 0;
	range->stop = 0;

	return(range);
}

void range_init(range_t *range) {
	range->yielded = false;
	range->value = range->start;
}

void range_set_stop(range_t *range, unsigned int const stop) {
	range->stop = stop;
}

void range_set_start(range_t *range, unsigned int const start) {
	range->start = start;
}

int range_next(range_t *range) {
	if ( range->yielded ) {
		range->value++;
	} else {
		range->yielded = true;
	}

	return( range->value < range->stop ? PC_SUCCESS : PC_ERROR_OVERFLOW );
}

void range_free(range_t **range) { 
	if ( *range != NULL ) {
		free(*range);
		*range = NULL;
	}
}

ranges_t *ranges_alloc(unsigned int const dimensions) { 
	int i;
	ranges_t *ranges = NULL;

	ranges = (ranges_t *)malloc(sizeof(ranges_t));

	if ( ranges == NULL ) {
		return(ranges);
	}

	ranges->dimensions = dimensions;
	ranges->ranges = (range_t **)malloc(sizeof(range_t *)*dimensions);

	if ( ranges->ranges == NULL ) {
		ranges_free(&ranges);
		return(ranges);
	}

	for ( i = 0; i < dimensions; i++ ) {
		ranges->ranges[i] = range_alloc();
		if ( ranges->ranges[i] == NULL ) {
			ranges_free(&ranges);
			return(ranges);
		}
	}

	return(ranges);
}

void ranges_init(ranges_t *ranges) {
	int i;

	for ( i = 0 ; i < ranges->dimensions; i++ ) {
		range_init(ranges->ranges[i]);
		if ( i != ranges->dimensions-1 ) {
			range_next(ranges->ranges[i]);
		}
	}	
}

void ranges_free(ranges_t **ranges) {
	int i;

	if ( *ranges != NULL ) {
		for ( i = 0; i < (*ranges)->dimensions; i++ ) {
			range_free(&((*ranges)->ranges[i]));
		}

		free((*ranges)->ranges);
		free(*ranges);
	}
}

int ranges_set_start(ranges_t *ranges, unsigned int const dimension, 
		unsigned int const start) {
	if ( dimension < ranges->dimensions ) {
		range_set_start(ranges->ranges[dimension], start);
		return(PC_SUCCESS);
	} else {
		return(PC_ERROR_INDEX);
	}
}

void ranges_set_starts(ranges_t *ranges, unsigned int const start) {
	int i; 

	for ( i = 0; i < ranges->dimensions; i++ ) {
		range_set_start(ranges->ranges[i], start);
	}
}

int ranges_set_stop(ranges_t *ranges, unsigned int const dimension, 
		unsigned int const stop) {
	if ( dimension < ranges->dimensions ) {
		range_set_stop(ranges->ranges[dimension], stop);
		return(PC_SUCCESS);
	} else {
		return(PC_ERROR_INDEX);
	}
}

void ranges_set_stops(ranges_t *ranges, unsigned int const stop) {
	int i; 

	for ( i = 0; i < ranges->dimensions; i++ ) {
		range_set_stop(ranges->ranges[i], stop);
	}
}

int ranges_next(ranges_t *ranges) {
	int i;

	for ( i = ranges->dimensions - 1; i >= 0 ; i-- ) {
		if ( range_next(ranges->ranges[i]) == PC_SUCCESS ) {
			return(PC_SUCCESS);
		} else {
			range_init(ranges->ranges[i]);
			range_next(ranges->ranges[i]);
		}
	}

	return(PC_ERROR_OVERFLOW);
}

int ranges_fprintf(FILE *stream_out, ranges_t const *ranges) {
	int i;

	for ( i = 0; i < ranges->dimensions; i++ ) {
		fprintf(stream_out, "%u", ranges->ranges[i]->value);

		if ( i != ranges->dimensions-1 ) {
			fprintf(stream_out,  ",");
		}
	}

	fprintf(stream_out, "\n");

	return( ferror(stream_out) ? PC_ERROR_IO : PC_SUCCESS );
}
