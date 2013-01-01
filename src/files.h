#ifndef FILES_H_
#define FILES_H_

#include <stdio.h>

int stream_open(FILE **stream, FILE *default_stream, 
		char *filename, char *mode);
void stream_close(FILE *stream, FILE *default_stream);

int open_streams(FILE **stream_in, char *in_filename,
		FILE **stream_out, char *out_filename);
void free_streams(FILE *stream_in, FILE *stream_out);

#endif
