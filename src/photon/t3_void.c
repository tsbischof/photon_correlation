#include "t3_void.h"

/* 
 * Functions to implement t3 photon read/write.
 */
int t3v_fscanf(FILE *stream_in, void *photon) {
	return(t3_fscanf(stream_in, photon));
}

int t3v_fprintf(FILE *stream_out, void const *photon) {
	return(t3_fprintf(stream_out, photon));
}

long long t3v_window_dimension(void const *photon) {
	return(((t3_t *)photon)->pulse);
}

long long t3v_channel_dimension(void const *photon) {
	return(((t3_t *)photon)->channel);
}

/*void t3v_offset(void *record, offsets_t const *offsets) {
	if ( offsets->offset_time ) {
		((t3_t *)record)->time += 
				offsets->time_offsets[((t3_t *)record)->channel];
	}	
	
	if ( offsets->offset_pulse ) {
		((t3_t *)record)->pulse +=
				offsets->pulse_offsets[((t3_t *)record)->channel];
	}
}*/
