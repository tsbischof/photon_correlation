#include "test.h"
#include "range.h"
#include "../error.h"

int combinatorics_test(FILE *stream_in, FILE *stream_out,
		pc_options_t const *options) {
	ranges_t *ranges = ranges_alloc(options->channels);

	if ( ranges == NULL ) {
		return(PC_ERROR_MEM);
	}

	ranges_init(ranges);
	ranges_set_stops(ranges, options->channels);

	while ( ranges_next(ranges) == PC_SUCCESS ) {
		ranges_fprintf(stream_out, ranges);
	}

	ranges_free(&ranges);
	return(PC_SUCCESS);
}
