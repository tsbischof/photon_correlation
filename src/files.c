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
