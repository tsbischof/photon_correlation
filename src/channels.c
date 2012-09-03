#include "channels.h"

#include "error.h"
#include "modes.h"
#include "t2.h"
#include "t3.h"

int channels_dispatch(FILE *in_stream, FILE *out_stream, options_t *options) {
	debug("Channels!\n");

	if ( options->mode == MODE_T2 ) {
		return(channels_t2(in_stream, out_stream, options));
	} else if ( options->mode == MODE_T3 ) {
		return(channels_t3(in_stream, out_stream, options));
	} else {
		error("Mode not supported: %s\n", options->mode_string);
		return(-1);
	}
}

/* These routines are designed to implement a sorted list. The incoming entries
 * are already mostly sorted, so we only need to populate the list long enough
 * to have more entries than needed.
 * 
 * For exapmple, with offsets, once the distance between the first and last
 * photons is greater than the largest difference in offsets, the first photon
 * can be emitted. This continues until the distance is too small, at which 
 * point the next photon is grabbed.
 *
 * To do this, a fixed linear array is allocated as the buffer, and the correct
 * position in the list is determined by a binary search of the elements. The 
 * right-most elements are moved back and the new one inserted to keep the array
 * sorted. If this surpasses the limits of the array, a warning is thrown, but
 * we can allocate a large array to keep this from happening. As space clears
 * up at the beginning of the list (due to emission of elements), the remaining
 * members are shifted to the beginning.
 * 
 * Once the element is added, the end points are checked and the earliest 
 * members emitted as needed. Once these are cleared up, the process begins
 * again.
 *
 * Thus the algorithm looks something like:
 * 
 * sorted_photons = list()
 * for photon in filter(lambda x: not suppress(x), photons):
 *     photon.offset(offsets)
 *     sorted_insert(sorted_photons, photon)
 *     for out_photon in emit_photons(sorted_photons):
 *         yield(out_photon)
 * 
 * for photon in sorted_photons:
 *     yield(photon)
 *
 * In C, the implementation of each step is roughly:
 * suppress: options->suppress_channels, options->suppressed_channels
 * sorted_insert: Generic sorted insert by bsearch, memmove on an array, with
 *       some logic to check bounds and update parameters.
 * emit_photons: Takes min/max of offsets to check whether the bounds are 
 *       exceeded sufficiently to emit photons, then yields those photons.
 */

<<<<<<< HEAD
int sorted_insert(sorted_photons_t *photon_stream, photon_t,
		photon, photon_comparator) { 
	int index;
=======
	while ( ! next_t2(in_stream, &record) ) {
		if ( options->suppress_channels && 
			options->suppressed_channels[record.channel]) {
			;
		} else {
			print_t2(out_stream, &record, options);
		}
	}

	return(0);
}

int channels_t3(FILE *in_stream, FILE *out_stream, options_t *options) {
	t3_t record;
	
	while ( ! next_t3(in_stream, &record) ) {
		if ( options->suppress_channels && 
			options->suppressed_channels[record.channel]) {
			;
		} else {
			print_t3(out_stream, &record, options);
		}
	}
>>>>>>> 6db0155ce7cde323c46ce2e2f2f16e273f89ebc7

	index = bsearch(photon_stream->photons, photon, photon_comparator);
	if not valid_index:
 		return(-1);
	else: 
		check bounds 
		memmove photons
		insert photon
		update bounds
		return(0);
}
