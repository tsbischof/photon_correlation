#ifndef PICOQUANT_H_
#define PICOQUANT_H_

/* The original code is designed for a 32-bit system, so make sure that the 
 * bytes line up correctly by forcing the 32-bit memory structure.
 */

#include <stdio.h>
#include "types.h"

#define	PQ_SUCCESS	0
#define PQ_FAILURE -1
#define	PQ_READ_ERROR	-2
#define	PQ_MEM_ERROR	-3
#define PQ_EOF		-4
#define PQ_MODE_ERROR	-5
#define PQ_FILE_ERROR	-6
#define PQ_VERSION_ERROR -7

#pragma pack(4)
typedef struct {
	char *in_filename;
	char *out_filename;
	char *file_type_string;
	int file_type;
	int number;
	int print_header;
	int print_every;
	int binary_out;
	int print_resolution;
} pq_options_t;

typedef struct {
	char Ident[16];
	char FormatVersion[6];
} pq_header_t;

typedef int (*pq_dispatch_t)(FILE *, FILE *, pq_header_t *, pq_options_t *);

void pq_usage(void);

pq_dispatch_t pq_get_dispatch(pq_options_t *options, pq_header_t *pq_header);
int pq_header_read(FILE *in_stream, pq_header_t *pq_header);
void pq_header_print(FILE *out_stream, pq_header_t *pq_header);

void pq_print_t2(FILE *out_stream, long long int count,
		unsigned int channel, 
		long long int base_time, unsigned int record_time,
		double record_resolution, pq_options_t *options);
void pq_print_t3(FILE *out_stream, long long int count,
		unsigned int channel,
		long long int base_nsync, unsigned int record_nsync,
		unsigned int record_dtime, double record_resolution,
		pq_options_t *options);
void pq_print_tttr(FILE *out_stream, long long int count,
		unsigned int histogram_channel, int n_histogram_channels,
		long long int base_time, unsigned int record_time,
		pq_options_t *options);
void pq_print_interactive(FILE *out_stream, int curve, double time, 
		int counts, pq_options_t *options);
void print_status(long long int count, pq_options_t *options);
	
void pq_usage(void);

#endif
