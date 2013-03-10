#include "queue.h"
#include "t2.h"
#include "t3.h"
#include "../modes.h"

vector_t *photon_queue_alloc(int const mode, size_t const length) {
	if ( mode == MODE_T2 ) {
		return(vector_alloc(sizeof(t2_t), length));
	} else if ( mode == MODE_T3 ) {
		return(vector_alloc(sizeof(t3_t), length));
	} else {
		return(NULL);
	}
}
