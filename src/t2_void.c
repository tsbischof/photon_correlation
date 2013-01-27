#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "t2_void.h"
#include "error.h"
#include "t2.h"

/* 
 * Functions to implement t2 photon read/write.
 */
int t2v_fread(FILE *stream_in, void *photon) {
	return(t2_fread(stream_in, photon));
}

int t2v_fscanf(FILE *stream_in, void *photon) {
	return(t2_fscanf(stream_in, photon));
}

int t2v_fprintf(FILE *stream_out, void const *photon) {
	return(t2_fprintf(stream_out, photon));
}

int t2v_fwrite(FILE *stream_out, void const *photon) {
	return(t2_fwrite(stream_out, photon));
}

int64_t t2v_window_dimension(void const *photon) {
	return(((t2_t *)photon)->time);
}

int64_t t2v_channel_dimension(void const *photon) {
	return(((t2_t *)photon)->channel);
}

void t2v_offset(void *record, offsets_t const *offsets) {
	if ( offsets->offset_time ) {
		((t2_t *)record)->time +=
				offsets->time_offsets[((t2_t *)record)->channel];
	} 
}
