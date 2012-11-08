#include "vector_g2.h"
#include "error.h"

void correlate_vector_g2(fraction_t *signal_0, fraction_t *signal_1, int length,
		vector_g2_t *g2) {
	/* Update the correlation, using the givens signals and their length. */
	int i;
	int delay;
	int left, right;

	for ( i = 0; i < g2->limits.bins; i++ ) {
		delay = g2->delays[i];
	
		/* Check to see that the vectors are long enough to support the delay. 
		 */
		if ( abs(delay) < length ) {
			/* Increment the intensity by adding the weighted product of the
			 * signals, such that the eventual fraction takes the value
			 * it would have, if the averaging had been done on the fly.
			 */
			g2->g2[i].sum += (signal_0[left].sum*signal_0[left].number)*
					(signal_1[right].sum*signal_1[right].number);
			g2->g2[i].number += signal_0[left].number*signal_1[right].number;
		}
	}
}

void print_vector_g2(FILE *out_stream, vector_g2_t *g2, options_t *options) {
	int i;
	float64_t intensity;

	if ( options->binary_out ) {
		error("Binary output not yet supported.\n");
	} else {
		for ( i = 0; i < g2->limits.bins; i++ ) {
			if ( g2->g2[i].number <= 0 ) {
				intensity = 0;
			} else {
				intensity = (float64_t)g2->g2[i].sum/
						(float64_t)g2->g2[i].number;
			}

			fprintf(out_stream, "%s", g2->name);
			fprintf(out_stream, ",%"PRIf64",%"PRIf64,
					g2->edges.bin_edges[i],
					g2->edges.bin_edges[i+1]);
			fprintf(out_stream, "%"PRId64"/%"PRId64","PRIf64"\n",
					g2->g2[i].sum, g2->g2[i].number, intensity);
	}
}

vector_g2_t *allocate_vector_g2(limits_t *limits, int scale) {
	vector_g2_t *g2 = NULL;
	int result = 0;
	int i;

	g2 = (vector_g2_t *)malloc(sizeof(vector_g2_t));
	
	if ( g2 == NULL ) {
		result = -1;
	} else {
		memcpy(&(g2->limits), limits, sizeof(limits_t));
		g2->edges = allocate_edges(limits->bins);
		g2->delays = (int *)malloc(limits->bins*sizeof(int));
		g2->g2 = (fraction_t *)malloc(limits->bins*sizeof(fraction_t));

		if ( g2->edges == NULL || g2->delays == NULL || g2->g2 == NULL ) {
			result = -1;
		} else {
			result = edges_from_limits(g2->edges, limits, scale);
		}

		if ( ! result ) {
			for ( i = 0; i < limits->bins; i++ ) {
				g2->g2[i].sum = 0;
				g2->g2[i].number = 0;
			}
		}
	}
			
	if ( result ) {
		free_vector_g2(&g2);
		g2 = NULL;
	}

	return(g2);
}

void free_vector_g2(vector_g2_t **g2) {
	if ( *g2 != NULL ) {
		free_edges(&(*g2)->edges);
		free(&(*g2)->delays);
		free(&(*g2)->g2);
	}
}
