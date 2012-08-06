#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "error.h"
#include "histogram_gn.h"

/*
 *
 * Tools for dealing with edge definitions.
 *
 */
edges_t *allocate_edges(int n_bins) {
	edges_t *edges = NULL;
	int result = 0;
	int i;

	edges = (edges_t *)malloc(sizeof(edges_t));

	if ( edges == NULL ) {
		result = -1;
	} else {
		edges->n_bins = n_bins;
		edges->bin_edges = (double *)malloc(sizeof(double)*
				(edges->n_bins+1)); 
		edges->print_label = 0;

		if ( edges->bin_edges == NULL ) {
			result = -1;
		} else {
			for ( i = 0; i <= edges->n_bins; i++ ) {
				edges->bin_edges[i] = 0;
			} 
		}
	}
		
	if ( result ) { 
		free_edges(&edges);
	}

	return(edges);
}

void free_edges(edges_t **edges) {
	if ( *edges != NULL ) {
		free((*edges)->bin_edges);
		free(*edges);
	}
}

int edges_get_index(edges_t *edges, long long int value) {
	/* Perform a binary search of the edges to determine which bin the value
	 * falls into. 
	 */
	int lower_index;
	int upper_index;;
	int middle_index;

	/* Replace this with bsearch from stdlib? */

	/* Check that the value lies within the lower and upper limits of the bins.
 	 */
	lower_index = 0;
	upper_index = edges->n_bins;
	debug("Starting binary search with (%d, %d)\n", lower_index, upper_index);
	if ( value < edges->bin_edges[0] ) {
		return(-1);
	} else if ( value > edges->bin_edges[edges->n_bins] ) {
		return(-1);
	}

	while ( upper_index - lower_index > 1) {
		debug("At (%d, %d)\n", lower_index, upper_index);
		middle_index = (upper_index + lower_index)/2;
		if ( value >= edges->bin_edges[middle_index] ) {
			lower_index = middle_index;
		} else {
			upper_index = middle_index;
		}
	}

	return(lower_index);
}

int edges_from_limits(edges_t *edges, limits_t *limits, int scale) {
	int i;
	double lower; 
	double upper;
	double bins;
	double width;

	if ( scale == SCALE_LINEAR ) {
		lower = (double)limits->lower;
		upper = (double)limits->upper;
		bins = (double)limits->bins;
		width = (upper - lower)/bins;
		debug("Edges have linear scale: (%lld, %d, %lld).\n",
				limits->lower, limits->bins, limits->upper);

		for ( i = 0; i <= limits->bins; i++ ) {
			edges->bin_edges[i] = lower + i*width;
		}
	} else if ( scale == SCALE_LOG || scale == SCALE_LOG_ZERO ) {
		/* We require that the lower and upper limits are greater than zero,
		 * to simplify matters. We also include the 0 time peak with the
		 * first bin.
		 */
		debug("Edges have log scale.\n");

		if ( limits->lower <= 0 || limits->upper <= 0 ) {
			error("Limits for a log scale histogram must be at least zero.\n");
			return(-1);
		} else {
			lower = log((double)limits->lower);
			upper = log((double)limits->upper);
			bins = (double)limits->bins;
			width = (upper - lower)/bins;
			debug("Log-scale bin width: %lf\n", width);
			for ( i = 0; i <= limits->bins; i++ ) {
				edges->bin_edges[i] = exp(lower + i*width);
			}
			
			if ( scale == SCALE_LOG_ZERO ) {
				edges->bin_edges[0] = 0; /* Include the zero-time peak. */
			}
		}
	} else {
		error("Could not create edges from the limits provided: "
				"(%lld, %d, %lld)\n", 
				limits->lower, limits->bins, limits->upper);
		return(-1);
	}

	if ( round(edges->bin_edges[0]) != (double)limits->lower ||
			round(edges->bin_edges[edges->n_bins]) != (double)limits->upper ) {
		if ( round(edges->bin_edges[0]) == 0
				&& round(edges->bin_edges[edges->n_bins]) == limits->upper
				&& scale == SCALE_LOG_ZERO ) {
			; /* Fine, we have the correct limits. */
		} else {
			error("Calculated bins' limits do not match the ones specified. "
					"Got (%.0lf, %.0lf) from (%lld, %lld).\n", 
					edges->bin_edges[0], edges->bin_edges[edges->n_bins],
					limits->lower, limits->upper);
			return(-1);
		}
	}

	return(0);
}

void print_edges(FILE *out_stream, edges_t *edges) {
	int i;

	for ( i = 0; i <= edges->n_bins; i++ ) {
		fprintf(out_stream, "%d,%lf\n", i, edges->bin_edges[i]);
	}
}


/*
 *
 * Tools for dealing with n-dimensional histograms.
 *
 */
gn_histogram_t *allocate_gn_histogram(int n_dimensions, edges_t **dimensions) {
	gn_histogram_t *histogram;
	int result = 0;
	int i;

	histogram = (gn_histogram_t *)malloc(sizeof(gn_histogram_t));
	if ( histogram == NULL ) {
		result = -1;
	} else {
		sprintf(histogram->histogram_label, "0");
		histogram->n_dimensions = n_dimensions;
		histogram->index_bases = (int *)malloc(sizeof(int)*
				n_dimensions);
		histogram->dimensions = (edges_t **)malloc(sizeof(edges_t *)*
				n_dimensions);
		
		if ( histogram->index_bases == NULL || histogram->dimensions == NULL ) {
			result = -1;
		} else {
			for ( i = 0; i < n_dimensions; i++ ) {
				histogram->dimensions[i] = dimensions[i];
				histogram->index_bases[i] = 0;
			}
			result = gn_histogram_make_index_bases(histogram);
			if( ! result ) {
				result = gn_histogram_make_n_bins(histogram);
				histogram->counts = (unsigned int *)malloc(sizeof(unsigned int)*
						histogram->n_bins);
				if ( histogram->counts == NULL ) {	
					result = -1;
				} else {
					for ( i = 0; i < histogram->n_bins; i++ ) {
						histogram->counts[i] = 0;
					}
				}
			}
		}
	}

	if ( result ) {
		debug("Error during histogram allocation, freeing scratch space.\n");
		free_gn_histogram(&histogram);
	}

	return(histogram);
}

void free_gn_histogram(gn_histogram_t **histogram) {
	if ( *histogram != NULL ) {
		free((*histogram)->index_bases);
		free((*histogram)->counts);
		free((*histogram)->dimensions);
		free(*histogram);
	}
}

int gn_histogram_make_index_bases(gn_histogram_t *histogram) {
	/* The index of the histogram array corresponding to a tuple of
 	 * (x0, x1, x2, ...x(n-1)) can be determined by treating each dimension
 	 * as the base of some heterogeneous-based number. This comes fromt the fact
 	 * that the histogram itself is an n-dimensional tensor, each dimension
 	 * of which is of finite order. So, flattening the dimensions means we are
 	 * interating over the possible indices, with limits determined by the 
 	 * order of a dimension:
 	 * (0, 0, 0)
 	 * (0, 0, 1)
 	 * (0, 0, 2)
 	 * (0, 1, 0)
 	 * ...
 	 * (4, 5, 3)
 	 *
 	 * for orders 4, 5, and 3 of three dimensions.
 	 *
 	 * As such, the index of an element of this tensor can be determined by 
 	 * summing over the number of lower-ordered elements:
 	 * (5*3)*x0 + (3)*x1 + x2
 	 *
 	 * or, in general:
 	 * \sum_{j=0}^{n-1}{(\product_{k=j+1}^{n-1}{order_{k}})*x_{j}}
 	 *
 	 * In the case that all dimensions are of the same order, this just reduces
 	 * to calcutating the value of a base-n number with digits defined by x.
 	 *
 	 * Since we will need to perform this calculation many times, this function
 	 * determines the successive products, since these are common to all
 	 * future calculations. 
 	 */
	int result = 0;
	int base = 1;
	int i;

	if ( histogram->index_bases == NULL ) {
		/* We need these to perform the calculation. */
		result = -1;
	} else {
		for ( i = histogram->n_dimensions-1; i >= 0; i-- ) {
			histogram->index_bases[i] = base;
			base *= histogram->dimensions[i]->n_bins;
			if ( i > 0 && (histogram->index_bases[i] 
							< histogram->index_bases[i-1]) ) {
				/* Integer overflow, so we need to return an error. */
				error("Integer overflow while computing the index bases for "
						"gn_histogram. Reduce the number of bins to avoid this "
						"error.\n");
				result = -1;
				i = histogram->n_dimensions;
			}
		}
	}

	return(result);
}

int gn_histogram_make_n_bins(gn_histogram_t *histogram) {
	int result = 0;
	long long int previous = 1;
	int i;

	if ( histogram->dimensions == NULL ) {
		/* We need these to perform the calculation. */
		result = -1;
	} else {
		histogram->n_bins = 1;

		for ( i = 0; i < histogram->n_dimensions; i++ ) {
			histogram->n_bins *= histogram->dimensions[i]->n_bins;
			if ( histogram->n_bins < previous ) {
				/* Integer overflow. */
				error("Integer overflow while computing the number of bins "
						"gn_histogram. Reduce the number of bins.\n");
				result = -1;
				i = histogram->n_dimensions;
			} else {
				previous = histogram->n_bins;
			}
		}
	}

	return(result);
}

int gn_histogram_get_index(gn_histogram_t *histogram, 
		long long int *values) {
	int index = 0;
	int i;

	debug("Getting histogram index from values.\n");
	debug("Dimensions: %d\n", histogram->n_dimensions);
	for ( i = 0; i < histogram->n_dimensions; i++ ) {
		debug("Value: %lld\n", values[i]);
		debug("Index base: %i\n", histogram->index_bases[i]);
		index += edges_get_index(histogram->dimensions[i], values[i])*
				histogram->index_bases[i];
		debug("Done\n");
	}

	if ( index < 0 || index > histogram->n_bins ) {
		error("Error while computing bin index for raw value: \n");
		fprintf(stderr, "(");
		for ( i = 0; i < histogram->n_dimensions; i++ ) {
			fprintf(stderr, "%lld,", values[i]);
		}
		fprintf(stderr, "\b)\n");
	
		if ( index < 0 ) {
			fprintf(stderr, "Got %d.\n", index);
		} else {
			fprintf(stderr, "Got %d, which exceeds the number of "
                            "available bins.\n", index);
		}
		return(-1);
	} else {
		return(index);
	}
}

int gn_histogram_get_index_from_indices(gn_histogram_t *histogram,
		int *indices) {
	int index = 0;
	int i;

	for ( i = 0; i < histogram->n_dimensions; i++ ) {
		debug("(Base, index): (%d, %d)\n", histogram->index_bases[i],
				indices[i]);
		index += histogram->index_bases[i]*indices[i];
	}

	if ( index < 0 || index > histogram->n_bins ) {
		error("Error while computing bin index for indices: \n");
		fprintf(stderr, "(");
		for ( i = 0; i < histogram->n_dimensions; i++ ) {
			fprintf(stderr, "%d,", indices[i]);
		}
		fprintf(stderr, "\b)\n");
	
		if ( index < 0 ) {
			fprintf(stderr, "Got %d.\n", index);
		} else {
			fprintf(stderr, "Got %d, which exceeds the number of "
                            "available bins.\n", index);
		}
		return(-1);
	} else {
		return(index);
	}
}


int gn_histogram_increment(gn_histogram_t *histogram,
		long long int *values) {
	int index;

	index = gn_histogram_get_index(histogram, values);

	if ( index >= 0 ) {
		debug("Incrementing index %d.\n", index);
		histogram->counts[index] += 1;
		return(0);
	} else {
		debug("Invalid index.\n");
		return(-1);
	}
}

void print_gn_histogram(FILE *out_stream, gn_histogram_t *histogram) {
	/* General outline:
	 * 1. Allocate space for indices.
	 * 2. Iterate over all possible sets of indices.
	 * 3. For each set, print:
	 *      base label, dimension 0 label, dimension 0 bin, dimension 1 label,
	 *      dimension 1 bin, ..., counts
	 *
	 * This is fine for t3-g1 printing, since we can make the dimension label 
	 * '\b\0' to remove it from being printed. This is also fine for t2-gn
	 * printing, because the dimensions are tied only to the channel. However,
	 * since t3-gn has both pulse and time dimensions, we need to print the
	 * two of them together. This can most simply be done by ordering the 
	 * dimensions as (pulse, time, pulse time,...) and labeling the time
	 * dimensions as '\b\0\'.
	 */
	int *indices = NULL;
	int done = 0;
	int i;
	int bin_index;

	indices = (int *)malloc(sizeof(int)*histogram->n_dimensions);

	if ( indices == NULL ) {
		error("Could not allocate memory for the indices to print the "
				"histogram.\n");
	} else {
		debug("Initializing indices.\n");
		for ( i = 0; i < histogram->n_dimensions; i++ ) {
			indices[i] = 0;
		}

		while ( ! done ) {
			debug("New indices.\n");
			fprintf(out_stream, "%s", histogram->histogram_label);
			for ( i = 0; i < histogram->n_dimensions; i++ ) {
				if ( histogram->dimensions[i]->print_label ) {
					debug("Printing the label.\n");
					fprintf(out_stream, ",%s", 
							histogram->dimensions[i]->dimension_label);
				}
				debug("Printing the edge for dimension %d and bin %d.\n",
						i, indices[i]);
				fprintf(out_stream, ",%.2lf,%.2lf",
						histogram->dimensions[i]->bin_edges[indices[i]],
						histogram->dimensions[i]->bin_edges[indices[i]+1]);
			}


			debug("Getting the index in the histogram.\n");
			bin_index = gn_histogram_get_index_from_indices(histogram, indices);
			debug("Histogram bin index: %d.\n", bin_index);
			if ( bin_index >=  0 ) {
				fprintf(out_stream, ",%u\n", histogram->counts[bin_index]);
				done = gn_histogram_next_index(histogram, indices);
			} else {
				error("Invalid bin index encountered while printing a "	
						"histogram: %d found, but %d was the limit.\n",
						bin_index, histogram->n_bins);
				done = -1;
			}
		}
	}
	free(indices);
}

int gn_histogram_next_index(gn_histogram_t *histogram, int *indices) {
	/* Iterate through the possible indices, with limits defined by the size
	 * of each dimension in the histogram. If the indices are valid, return 0.
	 * If we loop through and overflow, return -1.
	 */
	int i;

	for ( i = (histogram->n_dimensions-1); i >= 0; i-- ) {
		indices[i] = (indices[i] + 1) % histogram->dimensions[i]->n_bins;
		if ( indices[i] != 0 ) {
			i = 0;
		} else if ( i == 0 ) {
			/* Overflow of indices.
			 */
			return(-1);
		}
	}

	return(0);
}
