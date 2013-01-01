#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

#include "t3.h"

int main(int argc, char *argv[]) {
	options_t options;

	int result = 0;
	FILE *stream_in = NULL;
	FILE *t3_stream_in = NULL;
	FILE *stream_out = NULL;

	program_options_t program_options = {
		11,
"This program calculates the histogram of t3 photons for windowed of a fixed\n"
"width. This is useful for studying how the lifetime of an emissive state\n"
"changes over time, or to see how such behavior correlates with intensity\n"
"fluctuations.\n",
		{OPT_HELP, OPT_VERBOSE, OPT_FILE_IN,
				OPT_NUMBER, OPT_PRINT_EVERY,
				OPT_MODE, OPT_CHANNELS, OPT_ORDER,
				OPT_TIME, OPT_TIME_SCALE}};

	pid_t picoquant_pid = getpid();
	pid_t correlate_pid;
	pid_t histogram_pid;

	int picoquant_correlate_pipe[2];
	int correlate_histogram_pipe[2];

	int result = 0;

	int histogram_index = 0;
	char *out_filename_base;

	t3_windowed_stream_t *windowed_stream;

	result = parse_options(argc, argv, &options, &program_options);
	
	if ( ! result ) {
		/* 1. Choose the output name (file-out, file-in, then stdout)
		 * 2. Choose output number.
		 * 3. Set up the processes for picoquant, correlate, histogram
		 * 4. Open the histogram file, run through the window.
		 * 5. Open the next histogram file and repeat.
		 *
		 * In the future, try to write this such that we retain the correlate
		 * and histogram processes and memory, to avoid having to build them
		 * each time.
		 */
		/* Choose the output name. */
		if ( options.out_filename != NULL ) {
			out_filename_base = &(options.out_filename);
		} else if ( options.in_filename != NULL ) {
			out_filename_base = &(options.in_filename);
		} else {
			out_filename_base = NULL;
		}

		pipe(picoquant_correlate_pipe);
		if ( (correlate_pid = fork()) == -1 ) {
			perror("correlate fork");
			exit(1);
		}

		if ( getpid() == picoquant_pid ) {
			/* Create the raw photon stream. */
			if ( stream_open(&stream_in, ...); ) {
			
			stream_out = fdopen(picoquant_correlate_pipe[1]);
			close(picoquant_correlate_pipe[0]);

			picoquant_dispatch(stream_in, stream_out, &options);

			fclose(stream_out);
			stream_close(stream_in, stdin);
		} else {
			/* Create the correlate/histogram pair and start doing things. */
			pipe(correlate_histogram_pipe);
			close(picoquant_correlate_pipe[1]);

		}

	}

	return(parse_result(result));
}
