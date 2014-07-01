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

#include <math.h>
#include "conversions.h"
#include "../error.h"

int t3_as_t2(photon_t const *src, photon_t *dst) {
	dst->t2.channel = src->t3.channel;
	dst->t2.time = src->t3.pulse;

	return(PC_SUCCESS);
}

int t3_to_t2(photon_t const *src, photon_t *dst, double repetition_rate, 
		long long time_origin) {
	if ( repetition_rate == 0 || repetition_rate > 1e12 ) {
		return(PC_ERROR_ZERO_DIVISION);
	}

	dst->t2.channel = src->t3.channel;
	dst->t2.time = (long long)floor(src->t3.pulse * (1e12/repetition_rate)) 
			+ src->t3.time;
	dst->t2.time += time_origin;

	return(PC_SUCCESS);
}

int t2_to_t3(photon_t const *src, photon_t *dst, double repetition_rate,
		long long time_origin) {
	double fractpart, intpart;

	long long time;

	if ( repetition_rate == 0 || repetition_rate > 1e12 ) {
		return(PC_ERROR_ZERO_DIVISION);
	}

	dst->t3.channel = src->t2.channel;

	time = src->t2.time - time_origin;

/* Round the pulse number to the nearest integer. */
	fractpart = modf(time*1e-12*repetition_rate, &intpart);
	dst->t3.pulse = (long long)floor(intpart + 0.5);;
	dst->t3.time = (long long)floor(fractpart/repetition_rate*1e12+0.5);

	return(PC_SUCCESS);
}
