#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gn.h"
#include "types.h"
#include "error.h"
#include "modes.h"
#include "statistics/intensity.h"
#include "statistics/number.h"
#include "correlation/correlator.h"
#include "histogram/histogram_gn.h"

int gn(FILE *stream_in, FILE *stream_out, pc_options_t const *options) {
/* This program must perform:
 * 1. correlation/histogramming
 * 2. intensity (normal or binned)
 */
	int result = PC_SUCCESS;

	photon_stream_t *photon_stream;
	correlator_t *correlator;
	histogram_gn_t *histogram;
	intensity_photon_t *count_all;
	intensity_photon_t *intensity;
	photon_number_t *number;

	FILE *histogram_file = NULL;
	FILE *count_all_file = NULL;
	FILE *intensity_file = NULL;
	FILE *number_file = NULL;

	char *base_name;
	char *run_dir;
	char *histogram_filename;
	char *count_all_filename;
	char *intensity_filename;
	char *number_filename;

	long long min_time_distance = options->time_limits.lower < 0 ?
			 0 : (long long)floor(options->time_limits.lower);
	long long max_time_distance = (long long)floor(
			max(fabs(options->time_limits.lower), 
				fabs(options->time_limits.upper)));
	long long min_pulse_distance = options->pulse_limits.lower < 0 ?
			0 : (long long)floor(options->pulse_limits.lower);
	long long max_pulse_distance = (long long)floor(
			max(fabs(options->time_limits.lower),
				fabs(options->time_limits.upper)));

	if ( options->filename_out != NULL ) {
		base_name = options->filename_out;
	} else if ( options->filename_in != NULL ) {
		base_name =  options->filename_in;
	} else {
		error("Must specify an output or input filename.\n");
		result = PC_ERROR_OPTIONS;
	}

	if ( result == PC_SUCCESS ) {
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
		histogram_filename = malloc(sizeof(char)*(strlen(base_name)+1000));
		count_all_filename = malloc(sizeof(char)*(strlen(base_name)+1000));
		intensity_filename = malloc(sizeof(char)*(strlen(base_name)+1000));
		number_filename = malloc(sizeof(char)*(strlen(base_name)+1000));
	
		if ( photon_stream == NULL || correlator == NULL || 
				histogram == NULL || count_all == NULL ||
				intensity == NULL || number == NULL ||
				run_dir == NULL || histogram_filename == NULL || 
				count_all_filename == NULL || intensity_filename == NULL ) {
			result = PC_ERROR_MEM;
		}
	}

	if ( result == PC_SUCCESS ) {
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
					options->set_start_time, options->start_time,
					options->set_stop_time, options->stop_time);
		} else {
			photon_stream_set_windowed(photon_stream,
					options->bin_width,
					options->set_start_time, options->start_time,
					options->set_stop_time, options->stop_time);
			intensity_photon_init(intensity,
					false,
					options->bin_width,
					options->set_start_time, options->start_time,
					options->set_stop_time, options->stop_time);
		}

		if ( options->mode == MODE_T3 ) {
			photon_number_init(number);
		}
	}

	if ( result == PC_SUCCESS ) {
		sprintf(run_dir, "%s.g%u.run", options->filename_in, options->order);
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
		while ( result == PC_SUCCESS && ! photon_stream_eof(photon_stream) ) {
			correlator_init(correlator);
			histogram_gn_init(histogram);
			photon_number_init(number);

			if ( options->bin_width == 0 ) {
				sprintf(histogram_filename, "g%u", options->order);
			} else {
				sprintf(histogram_filename, "g%u.%020lld_%020lld",
						options->order,
						photon_stream->window.lower,
						photon_stream->window.upper);
			} 

			histogram_file = fopen(histogram_filename, "w");

			if ( histogram_file == NULL ) {
				error("Could not open %s for writing.\n", histogram_filename);
				result = PC_ERROR_IO;
			}

			if ( options->mode == MODE_T3 ) {
				photon_number_init(number);

				if ( options->bin_width == 0 ) {
					sprintf(number_filename, "number");
				} else {
					sprintf(number_filename, "number.%020lld_%020lld",
							photon_stream->window.lower,
							photon_stream->window.upper);
				}

				number_file = fopen(number_filename, "w");

				if ( number_file == NULL ) {
					error("Could not open %s for writing.\n", number_filename);
					result = PC_ERROR_IO;
				}
			}

			if ( result == PC_SUCCESS ) {
				debug("-----------Working on (%lld, %lld)-------------\n", 
						photon_stream->window.lower,
						photon_stream->window.upper);
				while ( photon_stream_next_photon(photon_stream) 
						== PC_SUCCESS ) {
					debug("Found photon.\n");
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
				}
		
				correlator_flush(correlator);
				while ( correlator_next(correlator) == PC_SUCCESS ) {
					histogram_gn_increment(histogram, 
							correlator->correlation);
				}

				if ( histogram_file != NULL ) {
					histogram_gn_fprintf(histogram_file, histogram);
					fclose(histogram_file);
				}

				if ( options->mode == MODE_T3 && number_file != NULL ) {
					photon_number_flush(number);
					photon_number_fprintf(number_file, number);
					fclose(number_file);
				}

				photon_stream_next_window(photon_stream);
			}
		}

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

	free(run_dir);
	free(histogram_filename);
	free(count_all_filename);
	free(intensity_filename);
	free(number_filename);

	count_all_file != NULL ? fclose(count_all_file) : 0 ;
	intensity_file != NULL ? fclose(intensity_file) : 0 ;
	
	return(PC_SUCCESS);
}
