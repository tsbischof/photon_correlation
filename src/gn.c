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

#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gn.h"
#include "types.h"
#include "error.h"
#include "modes.h"
#include "files.h"
#include "statistics/intensity.h"
#include "statistics/bin_intensity.h"
#include "statistics/number.h"
#include "correlation/photon_gn.h"

/* 
 * For correlation we typically need to join several operations together.
 * At minimum, we must send the photons to the correlator and the correlations
 * to the histogrammer, but for diagnostic purposes it is often desirable
 * to send photons to an intensity measurement or other routine. As such, this
 * procedure is meant to push photons to all of the different measurements
 * desired, such that a single pass through the stream is sufficient for 
 * performing all calculations. This in principle enables real-time processing
 * but in practice just makes the whole act of processing simpler.
 */

int gn_run(program_options_t *program_options, int const argc,
		char * const *argv) {
	int result = PC_SUCCESS;
	FILE *stream_in = NULL;
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
		debug("Opening stream in (%s).\n", options->filename_in);
		result = stream_open(&stream_in, stdin, options->filename_in, "r");
	}

	if ( result == PC_SUCCESS ) {
		debug("Dispatching.\n");
		gn(stream_in, NULL, options);
	}

	debug("Cleaning up.\n");
	pc_options_free(&options);
	stream_close(stream_in, stdin);

	return(pc_check(result));
}

int gn(FILE *stream_in, FILE *stream_out, pc_options_t const *options) {
	int result = PC_SUCCESS;

	long long photon_number = 0;

	photon_stream_t *photon_stream = NULL;
	photon_gn_t *gn = NULL;
	intensity_photon_t *count_all = NULL;
	intensity_photon_t *intensity = NULL;
	photon_number_t *number = NULL;

	unsigned long long bin_width;

	FILE *gn_file = NULL;
	FILE *count_all_file = NULL;
	FILE *intensity_file = NULL;
	FILE *number_file = NULL;
	FILE *options_file = NULL;

	char *base_name = NULL;
	char *run_dir = NULL;
	char *gn_filename = NULL;
	char *count_all_filename = NULL;
	char *intensity_filename = NULL;
	char *number_filename = NULL;

	debug("Initializing options\n");
	if ( result == PC_SUCCESS ) {
		if ( options->filename_out != NULL ) {
			debug("Using output filename.\n");
			base_name = options->filename_out;
		} else if ( options->filename_in != NULL ) {
			debug("Using input filename.\n");
			base_name =  options->filename_in;
		} else {
			error("Must specify an output or input filename.\n");
			result = PC_ERROR_OPTIONS;
		}
	}

	if ( result == PC_SUCCESS ) {
		debug("Allocating memory.\n");
		photon_stream = photon_stream_alloc(options->mode);
		gn = photon_gn_alloc(options->mode, options->order,
				options->channels, options->queue_size,
				&(options->time_limits), &(options->pulse_limits));
		count_all = intensity_photon_alloc(options->channels, options->mode);
		intensity = intensity_photon_alloc(options->channels, options->mode);
		number = photon_number_alloc(options->channels * 64);
	
		run_dir = malloc(sizeof(char)*(strlen(base_name)+128));
		gn_filename = malloc(sizeof(char)*16);
		count_all_filename = malloc((strlen("count_all")+1)*sizeof(char));
		intensity_filename = malloc((strlen("intensity")+1)*sizeof(char));
		number_filename = malloc((strlen("number.td")+1)*sizeof(char));

		if ( photon_stream == NULL || gn == NULL || count_all == NULL ||
				intensity == NULL || number == NULL ||
				run_dir == NULL || gn_filename == NULL || 
				count_all_filename == NULL || intensity_filename == NULL ) {
			result = PC_ERROR_MEM;
		}
	}

	if ( result == PC_SUCCESS ) {
		debug("Initializing intensity\n");
		intensity_photon_init(count_all,
				true,
				1,
				false, 0,
				false, 0);

		photon_stream_init(photon_stream, stream_in);

		if ( options->window_width == 0 ) {
			/* Perform a single calculation */
			photon_stream_set_unwindowed(photon_stream);

			bin_width = options->bin_width ? options->bin_width :
					DEFAULT_BIN_WIDTH(options->mode);
		} else {
			photon_stream_set_windowed(photon_stream,
					options->window_width,
					options->set_start, options->start,
					options->set_stop, options->stop);

			bin_width = options->bin_width ? options->bin_width :
					options->window_width;
		}

		intensity_photon_init(intensity,
				false,
				bin_width,
				options->set_start, options->start,
				options->set_stop, options->stop);
			
	}

	if ( result == PC_SUCCESS ) {
		sprintf(run_dir, "%s.g%u.run", base_name, options->order);
		if ( mkdir(run_dir, 
				S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) ) {
			error("Could not make run directory: %s.\n", run_dir);
			result = PC_ERROR_IO;
		}
	}

	if ( result == PC_SUCCESS ) {
		if ( chdir(run_dir) ) {
			error("Could not chdir to %s.\n", run_dir);
			result = PC_ERROR_IO;
		}
	}

	if ( result == PC_SUCCESS ) {
		options_file = fopen("options", "w");

		if ( options_file == NULL ) {
			error("Could not open options for writing.\n");
			result = PC_ERROR_IO;
		} else {
			pc_options_fprintf(options_file, options);
			fflush(options_file);
		}
	}

	/* Set up the files */ 
	debug("Opening files.\n");
	if ( result == PC_SUCCESS ) {	
		sprintf(count_all_filename, "count_all");
		count_all_file = fopen(count_all_filename, "w");

		if ( count_all_file == NULL ) {
			error("Could not open %s for writing.\n", count_all_filename);
			result = PC_ERROR_IO;
		} 
	} 

	if ( result == PC_SUCCESS ) {
		sprintf(intensity_filename, "intensity");
		intensity_file = fopen(intensity_filename, "w");

		if ( intensity_file == NULL ) {
			error("Could not open %s for writing.\n", intensity_filename);
			result = PC_ERROR_IO;
		} 
	}

	if ( result == PC_SUCCESS ) {
		debug("Opening possible time-dependent files.\n");
		if ( options->window_width == 0 ) {
			sprintf(gn_filename, "g%u", options->order);
		} else {
			sprintf(gn_filename, "g%u.td", options->order);
		} 

		gn_file = fopen(gn_filename, "w");

		if ( gn_file == NULL ) {
			error("Could not open %s for writing.\n", gn_filename);
			result = PC_ERROR_IO;
		}

		if ( options->mode == MODE_T3 ) {
			if ( options->window_width == 0 ) {
				sprintf(number_filename, "number");
			} else {
				sprintf(number_filename, "number.td");
			}

			number_file = fopen(number_filename, "w");

			if ( number_file == NULL ) {
				error("Could not open %s for writing.\n", number_filename);
				result = PC_ERROR_IO;
			}
		}
	}

	/* Start the actual calculation */
	if ( result == PC_SUCCESS ) {
		/* Write the bin information to file, if time-dependent */
		if ( options->window_width != 0 ) {
			debug("Handling time-dependent file headers.\n");
			photon_gn_init(gn);
			photon_gn_fprintf_bins(gn_file, gn, 2);
		}

		debug("Starting the calculation.\n");
		while ( result == PC_SUCCESS && ! photon_stream_eof(photon_stream) ) {
			photon_gn_init(gn);

			if ( options->window_width == 0 ) {
				photon_number_init(number, false, 0, false, 0);
			} else {
				photon_number_init(number,
						true, photon_stream->window.lower,
						true, photon_stream->window.upper);
			}

			debug("-----------Working on (%lld, %lld)-------------\n", 
					photon_stream->window.lower,
					photon_stream->window.upper);
			while ( photon_stream_next_photon(photon_stream) == PC_SUCCESS ) {
				pc_status_print("gn", photon_number++, options);

				photon_gn_push(gn, &(photon_stream->photon));
				intensity_photon_push(count_all, &(photon_stream->photon));
				intensity_photon_push(intensity, &(photon_stream->photon));

				while ( intensity_photon_next(intensity) == PC_SUCCESS ) {
					intensity_photon_fprintf(intensity_file, intensity);
				}

				if ( options->mode == MODE_T3 ) {
					photon_number_push(number, &(photon_stream->photon));
				}
			}
	
			debug("Window over.\n");

			photon_gn_flush(gn);

			if ( gn_file != NULL ) {
				if ( options->window_width == 0 ) {
					photon_gn_fprintf(gn_file, gn);
				} else {
					fprintf(gn_file, "%lld,%lld,",
							photon_stream->window.lower,
							photon_stream->window.upper);
					photon_gn_fprintf_counts(gn_file, gn);
				}
			}

			if ( options->mode == MODE_T3 && number_file != NULL ) {
				photon_number_flush(number);
				if ( options->window_width == 0 ) {
					photon_number_fprintf(number_file, number);
				} else {
					fprintf(number_file, "%lld,%lld,",
							photon_stream->window.lower,
							photon_stream->window.upper);
					photon_number_fprintf_counts(number_file, number);
				}
			}

			photon_stream_next_window(photon_stream);
		}

		number_file != NULL ? fclose(number_file) : 0;
		gn_file != NULL ? fclose(gn_file) : 0;

		if ( result == PC_SUCCESS ) {
			intensity_photon_flush(count_all);
			while ( intensity_photon_next(count_all) == PC_SUCCESS ) {
				intensity_photon_fprintf(count_all_file, count_all);
			}

			intensity_photon_flush(intensity);
			while ( intensity_photon_next(intensity) == PC_SUCCESS ) {
				intensity_photon_fprintf(intensity_file, intensity);
			}
		}
	}

	photon_stream_free(&photon_stream);
	photon_gn_free(&gn);
	intensity_photon_free(&count_all);
	intensity_photon_free(&intensity);
	photon_number_free(&number);

	free(run_dir);
	free(gn_filename);
	free(count_all_filename);
	free(intensity_filename);
	free(number_filename);

	count_all_file != NULL ? fclose(count_all_file) : 0 ;
	intensity_file != NULL ? fclose(intensity_file) : 0 ;
	options_file != NULL ? fclose(options_file) : 0;
	
	return(result);
}
