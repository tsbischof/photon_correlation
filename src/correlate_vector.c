#include "correlate_vector.h"
#include "intensity.h"
#include "error.h"

/* Tools to read in an correlate a vector of values, rather than photons. This 
 * probably exists in other libraries, but it would be worthwhile to have a
 * simple implementation here.
 */

int correlate_vector(FILE *in_stream, FILE *out_stream, options_t *options) {
	int result = 0;
	counts_t *counts;

	counts = allocate_counts(options->channels);

	if ( counts == NULL ) {
		result = -1;
	} else {
		while ( ! next_counts(in_stream, counts, options) ) {
			print_counts(out_stream, counts, options);
		}
	}

	debug("Freeing counts.\n");
	free_counts(&counts);
	return(result);
}

/* Perform the implementation by allocating a fixed queue of entries, as needed
 * to perform all levels of correlation. Have a bunch of pointers to these 
 * entries, such that, when two valid values are found, the correlation
 * at that delay can be incremented. Go through this until the stream runs
 * dry. 
 */
