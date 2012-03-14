#include <stdio.h>

#include "t2.h"

int next_t2(FILE *in_stream, t2_t *record) {
	int result;

	result = fscanf(in_stream, "%u,%lld",
			&(record->channel),
			&(record->time));
	
	return(result != 2);
}

void print_t2(FILE *out_stream, t2_t *record) {
	fprintf(out_stream, "%u,%lld", 
			record->channel,
			record->time);
}
