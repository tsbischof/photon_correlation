#include <unistd.h>
#include <sys/types.h>

#include "gn.h"
#include "error.h"

int gn_raw(FILE *in_stream, FILE *out_stream, options_t *options) {
	int result = 0;

//	FILE *histogram_stream;
//	FILE *intensity_stream;

//	int picoquant_pipe[2];
//	int channels_pipe[2];
	int photon_pipe[2]; // Photons pass through here, and may just be picoquant
						// or channels in disguise.
	
//	int intensity_pipe[2]; // intensity or bin_intensity, as may be the case.
//	int correlate_pipe[2];
//	int count_all_pipe[2]; // Need all counts for normalization.
//
//	pid_t picoquant_pid = getpid();
//	pid_t channels_pid, correlate_pid, histogram_pid, intensity_pid;

/* There are three pipelines and two stages to pass through here. The first
 * stage is the photon production, which occurs one of two ways:
 * 1. picoquant
 * 2. picoquant -> channels (when offsets/suppression is needed)
 *
 * The second stage takes three forms:
 * 1. a. correlate -> histogram (most correlations)
 *    b. histogram (g1 of t3)
 * 2. intensity (approximate normalization, and integration time calculation)
 * 3. (optional) bin_intensity (exact normalization)
 *
 *
 * As such, we need to produce these different paths when needed, by forking
 * and piping data through the appropriate subprocesses.
 */

	/* First, handle the photons. */
	debug("Producing photon pipes and forks.\n");
	pipe(photon_pipe);

	/* Next, fork to get the two analysis paths. */

	/* Set up the histogram path. */

	/* Set up the intensity path. */

	/* If desired, set up the bin_intensity path */

	return(result);
}
