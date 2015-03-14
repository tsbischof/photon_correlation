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

#include <stdio.h>
#include <string.h>

#include "error.h"
#include "modes.h"

int mode_parse(int *mode, char const *mode_string) { 
	if ( mode_string == NULL ) {
		*mode = MODE_UNKNOWN;
		return(PC_ERROR_MODE);
	} 

	if ( !strcmp("t2", mode_string) ) {
		debug("Found mode t2.\n");
		*mode = MODE_T2;
		return(PC_SUCCESS);
	} else if ( !strcmp("t3", mode_string) ) {
		debug("Found mode t3.\n");
		*mode = MODE_T3;
		return(PC_SUCCESS);
	} else if ( ! strcmp("vector", mode_string) ) {
		debug("Found mode vector.\n");
		*mode = MODE_VECTOR;
		return(PC_SUCCESS);
	} else if ( ! strcmp("as-t2", mode_string) ) {
		debug("Found mode as-t2.\n");
		*mode = MODE_AS_T2;
		return(PC_SUCCESS);
	} else {
		error("Mode not recognized: %s.\n", mode_string);
		*mode = MODE_UNKNOWN;
		return(PC_ERROR_MODE);
	}
}
