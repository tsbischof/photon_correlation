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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "error.h"
#include "limits.h"

int limits_parse(limits_t *limits, const char *str) {
	int result;

	debug("Parsing limits: %s.\n", str);

	if ( str == NULL ) {
		error("Fatal error, no limits specified.\n");
		return(-1);
	}

	result = sscanf(str, "%lf,%zu,%lf", 
				&(limits->lower), 
				&(limits->bins),
				&(limits->upper));

	if ( result != 3 ) {
		error("Limits could not be parsed: %s.\n"
				"The correct format is lower,bins,upper (no spaces).\n",
				str);
		return(-1);
	}

	if ( limits->lower >= limits->upper ) {
		error("Lower limit must be less than upper limit "
				"(%lf, %lf specified)\n", 
				limits->lower, limits->upper);
		return(-1);
	}

	if ( limits->bins <= 0 ) {
		error("Must have at least one bin.\n");
		return(-1);
	}

	return(0);
}

int limits_valid(limits_t const *limits) {
	return( limits->bins > 0 && limits->lower < limits->upper );
}

int scale_parse(int *scale, const char *str) {
	if ( str == NULL ) {
		*scale = SCALE_LINEAR;
	} else {
		if ( !strcmp(str, "log") ) {
			*scale = SCALE_LOG;
		} else if ( !strcmp(str, "linear") ) {
			*scale = SCALE_LINEAR;
		} else if ( !strcmp(str, "log-zero") ) {
			*scale = SCALE_LOG_ZERO;
		} else {
			*scale = SCALE_UNKNOWN;
			error("Scale specified but not recognized: %s\n", str);
		}
	}
	
	return(*scale == SCALE_UNKNOWN);
}
