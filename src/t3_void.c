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

int t3v_echo(FILE *stream_in, FILE *stream_out, int binary_in, int binary_out) {
	t3_t t3;
	photon_stream_t photons;
	photon_stream_next_t next = NULL;
	t3v_next_t photon_next = T3V_NEXT(binary_in);
	t3v_print_t print = T3V_PRINT(binary_out);

	photon_stream_init(&photons, &next, t3v_window_dimension, photon_next,
			sizeof(t3_t), stream_in, 
			0, 0, 0, 0, 0);

	while ( next(&photons, &t3) == PC_SUCCESS ) {
		print(stream_out, &t3);
	}

	return(PC_SUCCESS);
}

