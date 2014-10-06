/*
 * Copyright (c) 2011-2014, Thomas Bischof
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

#include "multi_tau.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../combinatorics/combinations.h"
#include "../error.h"

/* as a naive method, we want to keep the signal as a fixed n x m x p array.
 * However, since we are doing a lot of rotation of bins, it is worthwhile
 * to use pointers to array and shuffle those pointers. This saves the cost
 * of memmoves for each step (hopefully...).
 */
multi_tau_g2cn_t *multi_tau_g2cn_alloc(unsigned int const binning,
		unsigned int const registers, unsigned int const depth,
		unsigned int const channels, unsigned long long const bin_width) {
	int i, j, c0;
	int result = PC_SUCCESS;
	multi_tau_g2cn_t *mt = NULL;

	if ( binning > registers ) {
		error("Binning must be smaller than the number of registers.\n");
		result = PC_ERROR_OPTIONS;
	} else if ( binning == 0 ) {
		error("Binning must be non-zero.\n");
		result = PC_ERROR_OPTIONS;
	} else if ( channels == 0 ) {
		error("Number of channels must be non-zero.\n");
		result = PC_ERROR_OPTIONS;
	} else if ( depth == 0 ) {
		error("Depth must be non-zero.\n");
		result = PC_ERROR_OPTIONS;
	}

	if ( result == PC_SUCCESS ) {
		mt = (multi_tau_g2cn_t *)malloc(sizeof(multi_tau_g2cn_t));

		if ( mt == NULL ) {
			result = PC_ERROR_MEM;
		}
	}

	if ( result == PC_SUCCESS ) {
		mt->binning = binning;
		mt->registers = registers;
		mt->depth = depth;
		mt->channels = channels;

		mt->bin_width = bin_width;
	
		mt->intensity = (double *)malloc(sizeof(double)*mt->channels);
		mt->pushes = (unsigned long long *)malloc(sizeof(unsigned long long)
				*mt->depth);

		if ( mt->intensity == NULL || mt->pushes == NULL ) {
			error("Could not allocate intensity or pushes.\n");
			result = PC_ERROR_MEM;
		}
	}

	if ( result == PC_SUCCESS ) {
		mt->signal = (double ***)malloc(sizeof(double **)*mt->depth);
		if ( mt->signal == NULL ) {
			result = PC_ERROR_MEM;
		} 

		for ( i = 0; result == PC_SUCCESS && i < mt->depth; i++ ) {
			mt->signal[i] = (double **)malloc(sizeof(double *)*mt->registers);
			if ( mt->signal[i] == NULL ) {
				result = PC_ERROR_MEM;
			}

			for ( j = 0; result == PC_SUCCESS && j < mt->registers; j++ ) {
				mt->signal[i][j] = (double *)malloc(sizeof(double)*
						mt->channels);
				if ( mt->signal[i][j] == NULL ) {
					result = PC_ERROR_MEM;
				}
			}
		}
	}

	if ( result == PC_SUCCESS ) {
		mt->accumulated = (double **)malloc(sizeof(double *)*mt->depth);
		if ( mt->accumulated == NULL ) {
			result = PC_ERROR_MEM;
		} else {
			for ( i = 0; i < mt->depth; i++ ) {
				mt->accumulated[i] = (double *)malloc(sizeof(double)*
						mt->channels);
				if ( mt->accumulated[i] == NULL ) {
					result = PC_ERROR_MEM;
				}
			}
		}
	}

	if ( result == PC_SUCCESS ) {
		mt->averages = (double **)malloc(sizeof(double)*mt->depth);
		if ( mt->averages == NULL ) {
			result = PC_ERROR_MEM;
		} else {
			for ( i = 0; i < mt->depth; i++ ) {
				mt->averages[i] = (double *)malloc(sizeof(double)*
						mt->channels);
				if ( mt->averages[i] == NULL ) {
					result = PC_ERROR_MEM;
				}
			}
		}
	}

	if ( result == PC_SUCCESS ) {
		mt->g2 = (double ****)malloc(sizeof(double ***)*mt->depth);
		if ( mt->g2 == NULL ) {
			result = PC_ERROR_MEM;
		}

		for ( i = 0; result == PC_SUCCESS && i < mt->depth; i++ ) {
			mt->g2[i] = (double ***)malloc(sizeof(double **)*mt->registers);
			if ( mt->g2[i] == NULL ) {
				result = PC_ERROR_MEM;
			}

			for ( j = 0; result == PC_SUCCESS && j < mt->registers; j++ ) {
				mt->g2[i][j] = (double **)malloc(sizeof(double *)*mt->channels);
				if ( mt->g2[i][j] == NULL ) {
					result = PC_ERROR_MEM;
				}

				for ( c0 = 0; result == PC_SUCCESS && c0 < mt->channels; 
						c0++ ) {
					mt->g2[i][j][c0] = (double *)malloc(sizeof(double)*
							mt->channels);
					if ( mt->g2[i][j][c0] == NULL ) {
						result = PC_ERROR_MEM;
					}
				}
			}
		}
	}

	if ( result == PC_SUCCESS ) {
		mt->tau = (unsigned long long **)malloc(sizeof(unsigned long long *)*
				mt->depth);
		if ( mt->tau != NULL ) {
			for ( i = 0; result == PC_SUCCESS && i < mt->depth; i++ ) {
				mt->tau[i] = (unsigned long long *)malloc(
						sizeof(unsigned long long)*mt->registers);
				if ( mt->tau[i] == NULL ) {
					result = PC_ERROR_MEM;
				} else {
					for ( j = 0; j < mt->registers; j++ ) {
						mt->tau[i][j] = j*powull(mt->binning, i);
					}
				}
			}
		}
	}

	if ( result != PC_SUCCESS ) {
		multi_tau_g2cn_free(&mt);
		mt = NULL;
	} 
	
	return(mt);
}

void multi_tau_g2cn_init(multi_tau_g2cn_t *mt) {
	int i, j, c0;

	mt->n_seen = 0;

	memset(mt->intensity, (double)0, sizeof(double)*mt->channels);
	memset(mt->pushes, 0, sizeof(unsigned int)*mt->depth);

	for ( i = 0; i < mt->depth; i++ ) {
		memset(mt->accumulated[i], 0, sizeof(double)*mt->channels);
		memset(mt->averages[i], 0, sizeof(double)*mt->channels);
		
		for ( j = 0; j < mt->registers; j++ ) {
			memset(mt->signal[i][j], 0, sizeof(double)*mt->channels);

			for ( c0 = 0; c0 < mt->channels; c0++ ) {
				memset(mt->g2[i][j][c0], 0, sizeof(double)*mt->channels);
			}
		}
	}
}

void multi_tau_g2cn_free(multi_tau_g2cn_t **mt) {
	int i, j, c0;

	if ( *mt != NULL ) {
		free((*mt)->intensity);
		free((*mt)->pushes);

		for ( i = 0; (*mt)->signal != NULL && i < (*mt)->depth; i++ ) {
			for ( j = 0; (*mt)->signal[i] != NULL && j < (*mt)->registers; 
					j++ ) {
				free((*mt)->signal[i][j]);
			}
			free((*mt)->signal[i]);
		}
		free((*mt)->signal);

		for ( i = 0; (*mt)->accumulated != NULL && i < (*mt)->depth; i++ ) {
			free((*mt)->accumulated[i]);
		}
		free((*mt)->accumulated);

		for ( i = 0; (*mt)->averages != NULL && i < (*mt)->depth; i++ ) {
			free((*mt)->averages[i]);
		}
		free((*mt)->averages);

		for ( i = 0; (*mt)->g2 != NULL && i < (*mt)->depth; i++ ) {
			for ( j = 0; (*mt)->g2[i] != NULL && j < (*mt)->registers; j++ ) {
				for ( c0 = 0; (*mt)->g2[i][j] != NULL && c0 < (*mt)->channels; 
						c0++ ) {
					free((*mt)->g2[i][j][c0]);
				}
				free((*mt)->g2[i][j]);
			}
			free((*mt)->g2[i]);
		}
		free((*mt)->g2);

		for ( i = 0; (*mt)->tau != NULL && i < (*mt)->depth; i++ ) {
			free((*mt)->tau[i]);
		}
		free((*mt)->tau);

		free(*mt);
		*mt = NULL;
	}
}

void multi_tau_g2cn_push(multi_tau_g2cn_t *mt, counts_t const *counts) {
	int i, j, c0, c1;
	double *temp;

	mt->n_seen++;

	debug("Copying to buffer.\n");

	for ( c0 = 0; c0 < mt->channels; c0++ ) {
		mt->intensity[c0] = (double)counts->counts[c0];
	}

	/* update signal */
	debug("Updating signal.\n");
	for ( i = 0; i < mt->depth; i++ ) {
		temp = mt->signal[i][mt->registers-1];

		memmove(&(mt->signal[i][1]),
				&(mt->signal[i][0]),
				sizeof(double *)*(mt->registers-1)); 
		mt->signal[i][0] = temp; 

		memcpy(mt->signal[i][0], mt->intensity, sizeof(double)*mt->channels);
	
		for ( c0 = 0; c0 < mt->channels; c0++ ) {
			mt->accumulated[i][c0] += mt->intensity[c0];
		}

		mt->pushes[i]++;
		/* use this section to update the correlation, since this row just
		 * saw a push 
		 */
		for ( c0 = 0; c0 < mt->channels; c0++ ) {
			mt->averages[i][c0] += mt->signal[i][0][c0];

			for ( c1 = 0; c1 < mt->channels; c1++ ) {
				for ( j = (i == 0 ? 0 : mt->registers/mt->binning); 
						j < mt->registers && j < mt->pushes[i]; j++ ) {
					mt->g2[i][j][c0][c1] += 
							mt->signal[i][0][c0]*
							mt->signal[i][j][c1];
				}
			}
		}

		if ( mt->pushes[i] % mt->binning == 0 ) {
			for ( c0 = 0; c0 < mt->channels; c0++ ) {
				mt->intensity[c0] = mt->accumulated[i][c0]/mt->binning;
				mt->accumulated[i][c0] = 0;
			}
		} else {
			break;
		}
	}

	/* update correlation */
	/* use this section to update all correlations.
	debug("Updating correlation.\n");
	for ( i = 0; i < mt->depth; i++ ) {
		for ( c0 = 0; c0 < mt->channels; c0++ ) {
			mt->averages[i][c0] += mt->signal[i][0][c0];

			debug("g2.\n");
			for ( c1 = 0; c1 < mt->channels; c1++ ) {
				for ( j = (i == 0 ? 0 : mt->registers/mt->binning) ;
						j < mt->registers; j++ ) {
					mt->g2[i][j][c0][c1] += mt->signal[i][0][c0]*
							mt->signal[i][j][c1];
				}
			}
		}
	}*/
}

int multi_tau_g2cn_fprintf(FILE *stream_out, multi_tau_g2cn_t const *mt) {
	unsigned int i, j, c0, c1;
	double normalization;
	double correlation;

	for ( c0 = 0; c0 < mt->channels; c0++ ) {
		for ( c1 = 0; c1 < mt->channels; c1++ ) {
			for ( i = 0; i < mt->depth; i++ ) {
				for ( j = (i == 0 ? 0 : mt->registers/mt->binning);
						j < mt->registers; j++ ) {
					if ( mt->n_seen == 0 || mt->pushes[i] <=  j ) {
						normalization = 0;
					} else {
						normalization = 1;
						normalization *= mt->pushes[i];
						normalization *= (double)mt->averages[i][c0]/
								(double)mt->pushes[i];
						normalization *= (double)mt->averages[i][c1]/
								(double)mt->pushes[i]; 
						/* correct for the undersampling of various bins */
						normalization *= (double)(mt->pushes[i]-j)/
								(double)mt->pushes[i];
					}

					if ( normalization <= 0 ) {
						correlation = 0;
					} else {
						correlation = mt->g2[i][j][c0][c1]/normalization;
					}

					fprintf(stream_out,
							"%u,%u,%g,%g,%lf\n",
							c0, 
							c1,
							(double)mt->tau[i][j]*mt->bin_width,
							(double)(mt->tau[i][j]+pow_int(mt->binning, i))
									*mt->bin_width,
							correlation); 
				}
			}
		}
	}

	return( ferror(stream_out) ? PC_ERROR_IO : PC_SUCCESS );
}
