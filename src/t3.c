#include <stdio.h>

#include "t3.h"

int next_t3(FILE *in_stream, t3_t *record) {
	int result;

	result = fscanf(in_stream, "%u,%lld,%u",
			&(record->channel),
			&(record->pulse_number),
			&(record->time));
	
	return(result != 3);
}

void print_t3(FILE *out_stream, t3_t *record) {
	fprintf(out_stream, "%u,%lld,%u", 
			record->channel,
			record->pulse_number,
			record->time);
}
