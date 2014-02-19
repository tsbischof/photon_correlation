/*
 * Copyright (c) 2011-2014, Thomas Bischof
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
