#include <stdio.h>

#include "error.h"
#include "files.h"

int stream_open(FILE **stream, FILE *default_stream, 
		char *filename, char *mode) {
	if ( filename == NULL ) {
		*stream = default_stream;
	} else {
		debug("Opening %s for read.\n", filename);
		*stream = fopen(filename, mode);

		if ( *stream == NULL ) {
			error("Could not open %s for reading.\n", filename);
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
