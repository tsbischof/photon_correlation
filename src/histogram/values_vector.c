#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "values_vector.h"

values_vector_t *values_vector_alloc(unsigned int const length) {
	values_vector_t *vv = (values_vector_t *)malloc(sizeof(values_vector_t));

	if ( vv == NULL ) {
		return(vv);
	}

	vv->length = length;
	vv->values = (long long *)malloc(sizeof(long long)*length);
	
	if ( vv->values == NULL ) {
		values_vector_free(&vv);
		return(vv);
	} 

	return(vv);
}

void values_vector_init(values_vector_t *vv) {
	memset(vv->values, 0, sizeof(long long)*vv->length);
}

void values_vector_free(values_vector_t **vv) {
	if ( *vv != NULL ) {
		free((*vv)->values); 
		free(*vv);
	}
}

long long values_vector_index(values_vector_t const *vv, 
		edges_t ** const edges) {
/* The values vector needs to be transformed into an index by:
 * 1. Extracting the index of each bin based on the edges.
 * 2. Turning those indices into an overall index, as with combinations.
 */
	int i;
	long long result;
	long long base = 1;
	long long index = 0;

	for ( i = vv->length-1; i >= 0; i-- ) {
		result = edges[i]->get_index(edges[i], vv->values[i]);

		if ( result < 0 ) {
			return(result);
		}

		index += base*result;
		base *= edges[i]->n_bins;
	}

	return(index);
}

