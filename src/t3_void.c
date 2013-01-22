#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "t3_void.h"
#include "error.h"
#include "t3.h"

/* 
 * Functions to implement t3 photon read/write.
 */
int t3v_fread(FILE *stream_in, void *photon) {
	return(t3_fread(stream_in, photon));
}

int t3v_fscanf(FILE *stream_in, void *photon) {
	return(t3_fscanf(stream_in, photon));
}

int t3v_fprintf(FILE *stream_out, void const *photon) {
	return(t3_fprintf(stream_out, photon));
}

int t3v_fwrite(FILE *stream_out, void const *photon) {
	return(t3_fwrite(stream_out, photon));
}

int64_t t3v_window_dimension(void const *photon) {
	return(((t3_t *)photon)->pulse);
}

int64_t t3v_channel_dimension(void const *photon) {
	return(((t3_t *)photon)->channel);
}

