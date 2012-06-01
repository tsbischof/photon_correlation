#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

#include "strings.h"
#include "picoquant.h"
#include "picoharp.h"
#include "hydraharp.h"
#include "timeharp.h"
#include "error.h"
#include "files.h"
#include "t2.h"
#include "t3.h"

void pq_usage(void) {
	fprintf(stdout, 
"Usage: picoquant [-r] [-v] [-i file_in] [-o file_out]\n"
"                 [-p print_every] [-n number] [-b] [-z] [-t]\n"
"\n"
"       Version %d.%d\n"
"\n"
"        -i, --file-in: Input file. By default, this is stdin.\n"
"       -o, --file-out: Output file. By default, this is stdout.\n"
"         -n, --number: Number of entries to process (most pertinent for\n"
"                       tttr modes). By default, processes all records.\n"
"    -p, --print-every: Print a status message after processing a specified\n"
"                       number of entries. By default, no status message\n"
"                       is printed.\n"
"        -v, --verbose: Print debug-level information.\n"
"    -r, --header-only: Print header information, but no entries. Useful for\n"
"                       debugging and checking file integrity.\n"
"     -b, --binary-out: Output a binary stream instead of ascii. Refer to\n"
"                       the documentation for each mode for the details of\n"
"                       each stream type. Generally, this will be identical\n"
"                       to the ascii mode in typing.\n"
"-z, --resolution-only: Print the time resolution of the measurement in \n"
"                       picoseconds, then exit.\n"
"          -t, --to-t2: Convert a t3 file to a t2 file. This assumes the sync\n"
"                       channel is regular and consistent over the whole run.\n"
"           -h, --help: Print this message.\n"
"\n"
"The file type and version will be detected automatically from the file header."
"\n",
				VERSION_MAJOR, VERSION_MINOR);
}

pq_dispatch_t pq_get_dispatch(pq_options_t *options, pq_header_t *pq_header) {
	if ( ! strcmp(pq_header->Ident, "PicoHarp 300") ) {
		return(ph_dispatch);
	} else if ( ! strcmp(pq_header->Ident, "TimeHarp 200") ) {
		return(th_dispatch);
	} else if ( ! strcmp(pq_header->Ident, "HydraHarp") ) {
		return(hh_dispatch);
	} else {
		return(NULL);
	}
}

int pq_header_read(FILE *in_stream, pq_header_t *pq_header) {
	int result;
	result = fread(pq_header, sizeof(pq_header_t), 1, in_stream);
	if ( result == 1 ) {
		return(PQ_SUCCESS);
	} else {
		return(PQ_READ_ERROR);
	}
}

void pq_header_print(FILE *out_stream, pq_header_t *pq_header) {
	fprintf(out_stream, "Ident = %s\n", pq_header->Ident);
	fprintf(out_stream, "FormatVersion = %s\n", pq_header->FormatVersion);
}

void pq_print_t2(FILE *out_stream, long long int count,
		unsigned int channel, 
		long long int base_time, unsigned int record_time,
		pq_options_t *options) {
	t2_t record;

	record.channel = channel;
	record.time = base_time + (long long int)record_time;

	if ( options->binary_out ) {
		fwrite(&record, sizeof(t2_t), 1, out_stream);
	} else {
		fprintf(out_stream, "%u,%lld\n", record.channel, record.time);
	}

	print_status(count, options);
}

void pq_print_t3(FILE *out_stream, long long int count,
		unsigned int channel, 
		long long int base_nsync, unsigned int record_nsync,
		unsigned int record_dtime,
		pq_options_t *options) {
	t3_t record;

	record.channel = channel;
	record.pulse_number = base_nsync + (long long int)record_nsync;
	record.time = record_dtime;

	if ( options->binary_out ) {
		fwrite(&record, sizeof(t3_t), 1, out_stream);
	} else {
		fprintf(out_stream, "%u,%lld,%u\n",
				record.channel, record.pulse_number, record.time);
	}
	
	print_status(count, options);
}

void pq_print_tttr(FILE *out_stream, long long int count,
		unsigned int histogram_channel, int n_histogram_channels,
		long long int base_time, unsigned int record_time,
		pq_options_t *options) {
	/* This attempts to make the tttr record look t3-like. It is actually a
 	 * record of a 0->1 stop-start event, found in the histogram channel
 	 * at the time specified by the internal clock, but ultimately we can treat
 	 * the clock tick as a sync pulse and go from there.
 	 */
	t3_t record;

	/* Channel is really 1, but we only have one data channel */
	record.channel = 0;
	record.pulse_number = base_time + (long long int)record_time;

	/* The histogram channels seem to count backwards, with an upper limit
	 * of n_histogram_channels.
	 */
	record.time = /*n_histogram_channels - */ histogram_channel;

	if ( options->binary_out ) {
		fwrite(&record, sizeof(t3_t), 1, out_stream);
	} else {
		fprintf(out_stream, "%u,%lld,%u\n", 
				record.channel, record.pulse_number,
				record.time);
	}

	print_status(count, options);
}

void print_status(long long int count, pq_options_t *options) {
	time_t rawtime;
	struct tm *timeinfo;
	char fmttime[80];

	if ( (options->print_every > 0) && 
			( (count % options->print_every) == 0 ) ) {
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(fmttime, 80, "%Y.%m.%d %H.%M.%S", timeinfo);
		fprintf(stderr, "%s: Record %20lld\n", fmttime, count);
	}
}

void pq_print_interactive(FILE *out_stream, int curve, double left_time,
		double right_time, int counts, pq_options_t *options) {
	if ( options->binary_out ) {
		fwrite(&time, 1, sizeof(time), out_stream);
		fwrite(&counts, 1, sizeof(counts), out_stream);
	} else {
		fprintf(out_stream, "%d,%.3lf,%.3lf,%d\n", curve, left_time,
				right_time, counts);
	}
}

void external_marker(FILE *out_stream, unsigned int marker, 
		pq_options_t *options) {
	fprintf(stderr, "External marker: %u\n", marker);
}

void print_resolution(FILE *out_stream, double resolution, 
		pq_options_t *options) {
	/* Given float representing the resolution in picoseconds, print the 
	 * appropriate value.
	 */
	fprintf(out_stream, "%e\n", resolution);
}

int main(int argc, char *argv[]) {
	/* This software is designed to read in Picoquant data files and
	 * translate them to ascii or other useful formats. It is a filter in
	 * the Unix sense, in that it is designed to accept a stream of raw data
	 * and output a stream of the minimally-processed data.
	 * 
	 * These file processing tools can also be used to build custom tools 
	 * for working with this data, such as gpu-enabled correlators or 
	 * wrappers for the data to other programming languages.
	 */
	pq_options_t options;
	pq_header_t pq_header;
	pq_dispatch_t pq_dispatch;

	int c;
	int option_index = 0;

	int result = PQ_SUCCESS;

	FILE *in_stream = NULL;
	FILE *out_stream = NULL;

	static struct option long_options[] = {
		{"file-in", required_argument, 0, 'i'},
		{"file-out", required_argument, 0, 'o'},
		{"header-only", no_argument, 0, 'r'},
		{"number", required_argument, 0, 'n'},
		{"print-every", required_argument, 0, 'p'},
		{"verbose", no_argument, 0, 'v'},
		{"resolution-only", no_argument, 0, 'z'},
		{"to-t2", no_argument, 0, 't'},
		{"binary-out", no_argument, 0, 'b'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}}; 

	/* Set some default values. */
	options.in_filename = NULL;
	options.out_filename = NULL;
	options.file_type_string = NULL;
	options.file_type = PQ_MODE_ERROR;
	options.number = INT_MAX;
	options.print_every = 0;
	options.print_header = 0;
	options.binary_out = 0;
	options.print_resolution = 0;
	options.to_t2 = 0;

	/* Parse the command-line options. */
	while ( (c = getopt_long(argc, argv, "hbi:o:rn:p:vzt", long_options,
				&option_index)) != -1 ) {
		switch (c) { 
			case 'h':
				pq_usage();
				return(0);
				break;
			case 'b':
				options.binary_out = 1;
				break;
			case 'i':
				options.in_filename = strdup(optarg);
				break;
			case 'o':
				options.out_filename = strdup(optarg);
				break;
			case 'r':
				options.print_header = 1;
				break;
			case 'n':
				options.number = atoi(optarg);
				break;
			case 'p':
				options.print_every = atoi(optarg);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'z':
				options.print_resolution = 1;
				break;
			case 't':
				options.to_t2 = 1;
				break;
			case '?':
				pq_usage();
				return(-1);
			default:
				pq_usage();
				return(-1);
		}
	}
	
	/* Perform sanity checks on the options. */
	result += stream_open(&in_stream, stdin, options.in_filename, "rb");
	result += stream_open(&out_stream, stdout, options.out_filename, "w");

	/* Do the actual work, if we have no errors. */
	if ( result == PQ_SUCCESS ) {
		result = pq_header_read(in_stream, &pq_header);
		if ( result != PQ_SUCCESS ) {
			error("Could not read string header.\n");
		}

		pq_dispatch = pq_get_dispatch(&options, &pq_header);
		if ( pq_dispatch == NULL ) {
			error("Could not identify board %s.\n", pq_header.Ident);
		} else {
			result = pq_dispatch(in_stream, out_stream, &pq_header, &options);
		}
	}
		
	/* Cleanup! */
	stream_close(in_stream, stdin);
	stream_close(out_stream, stdout);
	free(options.in_filename);
	free(options.out_filename);
	free(options.file_type_string);

	return(result);
}       

