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
#include "correlation/correlator.h"
#include "histogram/histogram_gn.h"

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
	correlator_t *correlator = NULL;
	histogram_gn_t *histogram = NULL;
	intensity_photon_t *count_all = NULL;
	intensity_photon_t *intensity = NULL;
	photon_number_t *number = NULL;
	bin_intensity_t *bin_intensity = NULL;

	FILE *histogram_file = NULL;
	FILE *count_all_file = NULL;
	FILE *intensity_file = NULL;
	FILE *number_file = NULL;
	FILE *bin_intensity_file = NULL;
	FILE *options_file = NULL;

	char *base_name = NULL;
	char *run_dir = NULL;
	char *histogram_filename = NULL;
	char *count_all_filename = NULL;
	char *intensity_filename = NULL;
	char *bin_intensity_filename = NULL;
	char *number_filename = NULL;

	long long min_time_distance;
	long long max_time_distance;
	long long min_pulse_distance;
	long long max_pulse_distance;

	debug("Initializing options\n");
	if ( result == PC_SUCCESS ) {
		min_time_distance = options->time_limits.lower < 0 ?
				 0 : (long long)floor(options->time_limits.lower);
		max_time_distance = (long long)floor(
				max(fabs(options->time_limits.lower), 
					fabs(options->time_limits.upper)));
		min_pulse_distance = options->pulse_limits.lower < 0 ?
				0 : (long long)floor(options->pulse_limits.lower);
		max_pulse_distance = (long long)floor(
				max(fabs(options->pulse_limits.lower),
					fabs(options->pulse_limits.upper)));
		debug("Limits: (%lld, %lld) , (%lld, %lld)\n",
				min_time_distance, max_time_distance,
				min_pulse_distance, max_pulse_distance);
	
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
		correlator = correlator_alloc(options->mode, options->order,
				options->queue_size, options->positive_only,
				min_time_distance, max_time_distance,
				min_pulse_distance, max_pulse_distance);
		histogram = histogram_gn_alloc(options->mode, options->order,
				options->channels,
				options->time_scale, &(options->time_limits),
				options->pulse_scale, &(options->pulse_limits));
		count_all = intensity_photon_alloc(options->channels, options->mode);
		intensity = intensity_photon_alloc(options->channels, options->mode);
		number = photon_number_alloc(options->channels * 64);
	
		run_dir = malloc(sizeof(char)*(strlen(base_name)+100));
		histogram_filename = malloc(sizeof(char)*1000);
		count_all_filename = malloc(sizeof(char)*1000);
		intensity_filename = malloc(sizeof(char)*1000);
		number_filename = malloc(sizeof(char)*1000);
		bin_intensity_filename = malloc(sizeof(char)*1000);

		if ( options->exact_normalization ) {
			bin_intensity = bin_intensity_alloc(options->mode, options->order,
					options->channels,
					&(options->time_limits), options->time_scale,
					&(options->pulse_limits), options->pulse_scale,
					options->queue_size);

			if ( bin_intensity == NULL ) {
				result = PC_ERROR_MEM;
			}
		}
	
		if ( photon_stream == NULL || correlator == NULL || 
				histogram == NULL || count_all == NULL ||
				intensity == NULL || number == NULL ||
				run_dir == NULL || histogram_filename == NULL || 
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

		if ( options->bin_width == 0 ) {
			photon_stream_set_unwindowed(photon_stream);
			intensity_photon_init(intensity,
					false,
					options->mode == MODE_T2 ? 50000000000 : 100000,
					options->set_start, options->start,
					options->set_stop, options->stop);
		} else {
			photon_stream_set_windowed(photon_stream,
					options->bin_width,
					options->set_start, options->start,
					options->set_stop, options->stop);
			intensity_photon_init(intensity,
					false,
					options->bin_width,
					options->set_start, options->start,
					options->set_stop, options->stop);
		}
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
		if ( options->bin_width == 0 ) {
			sprintf(histogram_filename, "g%u", options->order);
		} else {
			sprintf(histogram_filename, "g%u.td", options->order);
		} 

		histogram_file = fopen(histogram_filename, "w");

		if ( histogram_file == NULL ) {
			error("Could not open %s for writing.\n", histogram_filename);
			result = PC_ERROR_IO;
		}

		if ( options->exact_normalization ) {
			debug("Bin intensity.\n");
			if ( options->bin_width == 0 ) {
				sprintf(bin_intensity_filename, "bin_intensity");
			} else {
				sprintf(bin_intensity_filename, "bin_intensity.td");
			}

			bin_intensity_file = fopen(bin_intensity_filename, "w");
			if ( bin_intensity_file == NULL ) {
				error("Could not open %s for writing.\n",
						bin_intensity_filename);
				result = PC_ERROR_IO;
			}
		}

		if ( options->mode == MODE_T3 ) {
			if ( options->bin_width == 0 ) {
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

	/* Write the bin information to file, if time-dependent */
	if ( result == PC_SUCCESS && options->bin_width != 0 ) {
		debug("Handling time-dependent file headers.\n");
		histogram_gn_init(histogram);
		histogram_gn_fprintf_bins(histogram_file, histogram, 2);

		if ( options->exact_normalization ) {
			bin_intensity_init(bin_intensity,
					options->set_start, options->start,
					options->set_stop, options->stop);

			bin_intensity_fprintf_bins(bin_intensity_file, bin_intensity, 2);
		}
	}

	/* Start the actual calculation */
	if ( result == PC_SUCCESS ) {
		debug("Starting the calculation.\n");
		while ( result == PC_SUCCESS && ! photon_stream_eof(photon_stream) ) {
			histogram_gn_init(histogram);
			correlator_init(correlator);

			if ( options->bin_width == 0 ) {
				photon_number_init(number, false, 0, false, 0);
			} else {
				photon_number_init(number,
						true, photon_stream->window.lower,
						true, photon_stream->window.upper);
			}

			if ( options->exact_normalization ) {
				if ( options->bin_width == 0 ) {
					bin_intensity_init(bin_intensity,
							options->set_start, options->start,
							options->set_stop, options->stop);
				} else {
					bin_intensity_init(bin_intensity,
							true, photon_stream->window.lower,
							true, photon_stream->window.upper);
				}
			}

			debug("-----------Working on (%lld, %lld)-------------\n", 
					photon_stream->window.lower,
					photon_stream->window.upper);
			while ( photon_stream_next_photon(photon_stream) 
					== PC_SUCCESS ) {
				pc_status_print("gn", photon_number++, options);

				correlator_push(correlator, photon_stream->photon);
				intensity_photon_push(count_all, photon_stream->photon);
				intensity_photon_push(intensity, photon_stream->photon);

				while ( correlator_next(correlator) == PC_SUCCESS ) {
					histogram_gn_increment(histogram, 
							correlator->correlation);
				}

				while ( intensity_photon_next(intensity) == PC_SUCCESS ) {
					intensity_photon_fprintf(intensity_file, intensity);
				}

				if ( options->mode == MODE_T3 ) {
					photon_number_push(number, 
							(t3_t *)photon_stream->photon);
				}

				if ( options->exact_normalization ) {
					bin_intensity_push(bin_intensity, 
							photon_stream->photon);
				}
			}
	
			debug("Window over.\n");

			correlator_flush(correlator);
			while ( correlator_next(correlator) == PC_SUCCESS ) {
				histogram_gn_increment(histogram, 
						correlator->correlation);
			}

			if ( histogram_file != NULL ) {
				if ( options->bin_width == 0 ) {
					histogram_gn_fprintf(histogram_file, histogram);
				} else {
					fprintf(histogram_file, "%lld,%lld,",
							photon_stream->window.lower,
							photon_stream->window.upper);
					histogram_gn_fprintf_counts(histogram_file, histogram);
				}
			}

			if ( options->mode == MODE_T3 && number_file != NULL ) {
				photon_number_flush(number);
				if ( options->bin_width == 0 ) {
					photon_number_fprintf(number_file, number);
				} else {
					fprintf(number_file, "%lld,%lld,",
							photon_stream->window.lower,
							photon_stream->window.upper);
					photon_number_fprintf_counts(number_file, number);
				}
			}

			if ( options->exact_normalization && 
					bin_intensity_file != NULL ) {
				bin_intensity_flush(bin_intensity);
				if ( options->bin_width == 0 ) {
					bin_intensity_fprintf(bin_intensity_file, 
							bin_intensity);
				} else {
					fprintf(bin_intensity_file, "%lld,%lld,",
							photon_stream->window.lower,
							photon_stream->window.upper);
					bin_intensity_fprintf_counts(bin_intensity_file, 
							bin_intensity);
				}
			}

			photon_stream_next_window(photon_stream);
		}

		bin_intensity_file != NULL ? fclose(bin_intensity_file) : 0;
		number_file != NULL ? fclose(number_file) : 0;
		histogram_file != NULL ? fclose(histogram_file) : 0;

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
	correlator_free(&correlator);
	histogram_gn_free(&histogram);
	intensity_photon_free(&count_all);
	intensity_photon_free(&intensity);
	photon_number_free(&number);
	bin_intensity_free(&bin_intensity);

	free(run_dir);
	free(histogram_filename);
	free(count_all_filename);
	free(intensity_filename);
	free(number_filename);

	count_all_file != NULL ? fclose(count_all_file) : 0 ;
	intensity_file != NULL ? fclose(intensity_file) : 0 ;
	options_file != NULL ? fclose(options_file) : 0;
	
	return(PC_SUCCESS);
}
