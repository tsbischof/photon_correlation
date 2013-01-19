#include "bin_intensity.h"
#include "bin_intensity_photon.h"
#include "bin_intensity_t2.h"
#include "bin_intensity_t3.h"
#include "modes.h"
#include "error.h"

/* T2 and T3 are handled separately, but the general purpose and algorithm
 * is identical.
 *
 * This program calculates the intensity of a time-offset stream of photons.
 * The idea here is that we have a function I(t) which is defined for some
 * limits [lower, upper), and we have some time offset dt. We wish to calculate
 * I(t+dt), but the limits of integration have not changed. As such, any 
 * intensity which falls outside the limits of time is not counted. 
 *
 * The time offsets given here are the same as those for a histogram, such that
 * there is a start, number of bins, an end, and a scale.
 *
 * We must follow the intensity stream (photons), keeping track of whether or
 * not the photon falls into some histogram bin, based on that bin's offset
 * from the limits of integration. In the most general case, these limits
 * are defined by the first and last photon seen, but these can also be 
 * specified externally. If a photon is sufficiently far from the limits of
 * integration to be counted, it is, Otherwise, it is ignored.
 *
 * Thus there are three distinct classes of photons:
 * 1. Photon at the beginning of the stream (less far from the beginning than
 *    the longest delay permissible).
 * 2. Photon at the end of the stream (less far from the end than the longest
 *    delay permissible).
 * 3. Photon in the middle (all other photons).
 *
 * Graphically, this looks something like:
 * ends:        |<---------->|                              |<---------->|
 * photons:     *  *       *    * *          *        *                  *
 * class:       |<-  start ->|<---------- middle ---------->|<-- end  -->|
 *
 * As such, we must keep track of the current limits of integration time, 
 * and use those to determine whether the given time bin permits inclusion
 * of the given photon. The general algorithm will look something like:
 *
 * photon_queue = photons.pop()
 * max_delay = max(limits)
 * while photons:
 *     if photon_queue.back().time - photon_queue.front().time > max_delay:
 *         increment_bins(bins, photon_queue.pop(),
 *                        photon_queue.first_photon
 *                        photon_queue.back())
 *     else:
 *         photon_queue.push(photons.pop())
 *
 * while photon_queue:
 *     increment_bins(bins, photon_queue.pop(),
 *                    photon_queue.first_photon,
 *                    photon_queue.back())
 *
 * print_bins(bins)
 *
 *
 *
 * The logic behind the back-front>max_delay line is as follows: if a photon
 * is at least as far from the ends as the maximal displacement, it must be
 * able to be counted in any bin, and as such is ready for processing. This 
 * helps keep us from counting a photon as not included, when it really could
 * be when the next photon arrives.
 *
 * The increment process involves checking each bin to see whether the photon 
 * can be included, based on the first and last photons seen (or the start
 * and end of the integration time, as may be the case). Here the back() call
 * refers to the final photon, but in practice logic is used to check whether
 * the true time is the photon time or a specified time. The increment 
 * algorithm looks like:
 *
 * def increment_bins(bins, current_photon, front, back):
 *     for bin in bins:
 *         if (current_photon.time + bin.lower) in range(front, back)
 *             or (current_photon.time + bin.upper) in range(front, back):
 *             bin.increment()
 *
 *
 *
 * All of this is for T2 photons. The T3 pulse dimension is identical, but I
 * have not yet worked out how to deal with the time dimension. As such,
 * only the pulse dimension is worked out so far.
 */

int bin_intensity_dispatch(FILE *stream_in, FILE *stream_out, 
		options_t *options) {
	if ( options->use_void ) {
		return(bin_intensity_photon(stream_in, stream_out, options));
	} else if ( options->mode == MODE_T2 ) {
		return(bin_intensity_t2(stream_in, stream_out, options));
	} else if ( options->mode == MODE_T3 ) {
		return(bin_intensity_t3(stream_in, stream_out, options));
	} else {
		return(PC_ERROR_MODE);
	}
}

