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

int t2v_echo(FILE *stream_in, FILE *stream_out, int binary_in, int binary_out) {
	t2_t t2;
	photon_stream_t photons;
	photon_stream_next_t next = NULL;
	t2v_next_t photon_next = T2V_NEXT(binary_in);
	t2v_print_t print = T2V_PRINT(binary_out);

	photon_stream_init(&photons, &next, t2v_window_dimension, photon_next,
			sizeof(t2_t), stream_in, 
			0, 0, 0, 0, 0);

	while ( next(&photons, &t2) == PC_SUCCESS ) {
		print(stream_out, &t2);
	}

	return(PC_SUCCESS);
}

