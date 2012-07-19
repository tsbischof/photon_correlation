#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "picoquant.h"
#include "error.h"
#include "t2.h"
#include "t3.h"
#include "options.h"

#include "picoharp.h"
#include "hydraharp.h"
#include "timeharp.h"

int pq_dispatch(FILE *in_stream, FILE *out_stream, options_t *options) {
	int result;
	pq_header_t pq_header;
	pq_dispatch_t dispatch;

	/* Do the actual work, if we have no errors. */
	result = pq_header_read(in_stream, &pq_header);
	if ( result ) {
		error("Could not read string header.\n");
	} else {
		dispatch = pq_get_dispatch(options, &pq_header);
		if ( dispatch == NULL ) {
			error("Could not identify board %s.\n", pq_header.Ident);
		} else {
			result = dispatch(in_stream, out_stream, 
					&pq_header, options);
		}
	}

	return(result);
}       


pq_dispatch_t pq_get_dispatch(options_t *options, pq_header_t *pq_header) {
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
		options_t *options) {
	t2_t record;

	record.channel = channel;
	record.time = base_time + (long long int)record_time;

	if ( options->binary_out ) {
		fwrite(&record, sizeof(t2_t), 1, out_stream);
	} else {
		fprintf(out_stream, "%u,%lld\n", record.channel, record.time);
	}

	fflush(out_stream);

	print_status("picoquant", count, options);
}

void pq_print_t3(FILE *out_stream, long long int count,
		unsigned int channel, 
		long long int base_nsync, unsigned int record_nsync,
		unsigned int record_dtime,
		options_t *options) {
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

	fflush(out_stream);
	
	print_status("picoquant", count, options);
}

void pq_print_tttr(FILE *out_stream, long long int count,
		unsigned int histogram_channel, int n_histogram_channels,
		long long int base_time, unsigned int record_time,
		options_t *options) {
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

	print_status("picoquant", count, options);
}

void pq_print_interactive(FILE *out_stream, int curve, double left_time,
		double right_time, int counts, options_t *options) {
	if ( options->binary_out ) {
		fwrite(&time, 1, sizeof(time), out_stream);
		fwrite(&counts, 1, sizeof(counts), out_stream);
	} else {
		fprintf(out_stream, "%d,%.3lf,%.3lf,%d\n", curve, left_time,
				right_time, counts);
	}
	fflush(out_stream);
}

void external_marker(FILE *out_stream, unsigned int marker, 
		options_t *options) {
	fprintf(stderr, "External marker: %u\n", marker);
}

void print_resolution(FILE *out_stream, double resolution, 
		options_t *options) {
	/* Given float representing the resolution in picoseconds, print the 
	 * appropriate value.
	 */
	fprintf(out_stream, "%e\n", resolution);
}
