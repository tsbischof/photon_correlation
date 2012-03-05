#ifndef FILES_H_
#define FILES_H_

#include <stdio.h>

int stream_open(FILE **stream, FILE *default_stream, 
		char *filename, char *mode);
void stream_close(FILE *stream, FILE *default_stream);

#endif
