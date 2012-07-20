#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include <picoquant/picoquant.h>
#include <picoquant/correlate.h>
#include <picoquant/histogram.h>
#include <picoquant/files.h>
#include <picoquant/modes.h>
#include <picoquant/options.h>
#include <picoquant/error.h>

int main (int argc, char *argv[]) {
	pid_t picoquant_pid = getpid();
	pid_t correlate_pid;
	pid_t histogram_pid;

	FILE *in_stream, *out_stream;
	int picoquant_correlate_pipe[2];
	int correlate_histogram_pipe[2];

	options_t options;

	/* First, establish the options for the run. */
	default_options(&options);

	options.mode = MODE_T2;
	options.max_time_distance = 999999;

//	options.number = 0;

	options.in_filename = "/home/tsbischof/Documents/projects/picoquant/sample_data/picoharp/v20.pt2";
	options.out_filename = "blargh.hist";

	options.time_scale = SCALE_LINEAR;
	options.time_limits.lower = -options.max_time_distance-1;
	options.time_limits.bins = 20001;
	options.time_limits.upper = options.max_time_distance+1;

	options.print_every = 10000;
	verbose = (argc > 1);

	/* Next, spawn the correlate and histogram subprocesses. */

	pipe(picoquant_correlate_pipe);

	if ( (correlate_pid = fork()) == -1 ) {
		perror("Correlate fork");
		exit(1);
	} 

	if ( correlate_pid == 0 ) {
		pipe(correlate_histogram_pipe);

		if ( (histogram_pid = fork()) == -1 ) {
			perror("Histogram fork");
			exit(1);
		}
	}

	debug("PID: %d (%d, %d, %d)\n", getpid(), 
			picoquant_pid, correlate_pid, histogram_pid);
	/* Finally, run the processes. */
	if ( getpid() == picoquant_pid ) {
		/* Picoquant */
		debug("Picoquant!\n");

		if ( stream_open(&in_stream, stdin, options.in_filename, "r") ) {
			exit(1);
		} 

		out_stream = fdopen(picoquant_correlate_pipe[1], "w");
		close(picoquant_correlate_pipe[0]);

		pq_dispatch(in_stream, out_stream, &options);

		stream_close(in_stream, stdin);
		fclose(out_stream);
	} else if ( correlate_pid == 0 && histogram_pid != 0) {
		/* Correlate */
		debug("Correlate!\n");

		in_stream = fdopen(picoquant_correlate_pipe[0], "r");
		close(picoquant_correlate_pipe[1]);

		out_stream = fdopen(correlate_histogram_pipe[1], "w");
		close(correlate_histogram_pipe[0]);
		
		correlate_dispatch(in_stream, out_stream, &options);

		fclose(in_stream);
		fclose(out_stream);
	} else if ( histogram_pid == 0 ) {
		/* Histogram */
		debug("Histogram!\n");

		if ( stream_open(&out_stream, stdout, options.out_filename, "w") ) {
			exit(1);
		}

		close(picoquant_correlate_pipe[0]);
		close(picoquant_correlate_pipe[1]);

		in_stream = fdopen(correlate_histogram_pipe[0], "r");
		close(correlate_histogram_pipe[1]);

		histogram_dispatch(in_stream, out_stream, &options);

		fclose(in_stream);
		stream_close(out_stream, stdout);
	} else {
		error("PID not recognized\n");
	}

	return(0);
}
