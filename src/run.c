/*
 * Copyright (c) 2011-2015, Thomas Bischof
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Massachusetts Institute of Technology nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "run.h"
#include "error.h"
#include "files.h"

/*
 * Most programs follow a common routine for processing:
 * 1. Check options. 
 * 2. Initialize I/O
 * 3. Run dispatch to process data.
 * 4. Clean up.
 * 
 * This procedure implements this process.
 */

int run(program_options_t *program_options, dispatch_t const dispatch,
		int const argc, char * const *argv) {
	int result = PC_SUCCESS;
	FILE *stream_in = NULL;
	FILE *stream_out = NULL;
	pc_options_t *options = pc_options_alloc();

	if ( options == NULL ) {
		error("Could not allocate options.\n");
		return(PC_ERROR_MEM);
	}

	pc_options_init(options, program_options);
	result = pc_options_parse(options, argc, argv);

	if ( result != PC_SUCCESS || ! pc_options_valid(options)) {
		if ( options->usage ) {
			pc_options_usage(options, argc, argv);
			result = PC_USAGE;
		} else if ( options->version ) {
			pc_options_version(options, argc, argv);
			result = PC_VERSION;
		} else {
			debug("Invalid options.\n");
			result = PC_ERROR_OPTIONS;
		}
	}

	if ( result == PC_SUCCESS ) {
		debug("Opening streams.\n");
		result = streams_open(&stream_in, options->filename_in,
				&stream_out, options->filename_out);
	}

	if ( result == PC_SUCCESS ) {
		debug("Dispatching.\n");
		dispatch(stream_in, stream_out, options);
	}

	debug("Cleaning up.\n");
	pc_options_free(&options);
	streams_close(stream_in, stream_out);

	return(pc_check(result));
}
