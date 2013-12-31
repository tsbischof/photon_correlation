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

#include "error.h"
#include "files.h"

/* 
 * In each program, files must be opened and closed, either as references to
 * a file in the system or as stdin/stdout. By default, stdin/stdout are used,
 * so we put the logic to check whether we are opening a file or stream here.
 */

int stream_open(FILE **stream, FILE *default_stream, 
		char *filename, char *mode) {
	if ( filename == NULL ) {
		*stream = default_stream;
	} else {
		debug("Opening %s with mode %s.\n", filename, mode);
		*stream = fopen(filename, mode);

		if ( *stream == NULL ) {
			error("Could not open %s with mode %s.\n", filename, mode);
			return(-1);
		}
	}

	return(0);
}

void stream_close(FILE *stream, FILE *default_stream) { 
	if ( stream != NULL && stream != default_stream ) {
		fclose(stream);
	}
}

int streams_open(FILE **stream_in, char *in_filename,
		FILE **stream_out, char *out_filename) {
	return( stream_open(stream_in, stdin, in_filename, "r") +
			stream_open(stream_out, stdout, out_filename, "w") );
}

void streams_close(FILE *stream_in, FILE *stream_out) {
	stream_close(stream_in, stdin);
	stream_close(stream_out, stdout);
}
