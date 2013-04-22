#ifndef MULTI_TAU_H_
#define MULTI_TAU_H_

#include <stdio.h>

#include "../statistics/counts.h"

/* n-channel implementation of the multi-tau algorithm. For performance 
 * purposes, it may end up being worthwhile to write a separate implementation
 * which uses only one channel.
 */
typedef struct {
	unsigned int binning;
	unsigned int registers;
	unsigned int depth;

	unsigned int channels;
	unsigned long long bin_width;

	unsigned long long n_seen;

	double *intensity;

	double ***signal;
	double **accumulated;
	unsigned int *pushes;
	double **averages;

	double ****g2;
	unsigned long long **tau;
} multi_tau_g2cn_t;

multi_tau_g2cn_t *multi_tau_g2cn_alloc(unsigned int const binning,
		unsigned int const registers, unsigned int const depth,
		unsigned int const channels, unsigned long long const bin_width);
void multi_tau_g2cn_init(multi_tau_g2cn_t *mt);
void multi_tau_g2cn_free(multi_tau_g2cn_t **mt);

void multi_tau_g2cn_push(multi_tau_g2cn_t *mt, counts_t const *counts);

int multi_tau_g2cn_fprintf(FILE *stream_out, multi_tau_g2cn_t const *mt);

#endif
