#include <math.h>
#include "conversions.h"
#include "../error.h"

int t3_as_t2(t3_t const *t3, t2_t *t2) {
	t2->channel = t3->channel;
	t2->time = t3->pulse;

	return(PC_SUCCESS);
}

int t3_to_t2(t3_t const *t3, t2_t *t2, double repetition_rate, 
		long long time_origin) {
	if ( repetition_rate == 0 || repetition_rate > 1e12 ) {
		return(PC_ERROR_ZERO_DIVISION);
	}

	t2->channel = t3->channel;
	t2->time = (int64_t)floor(t3->pulse * (1e12/repetition_rate)) + t3->time;
	t2->time += time_origin;

	return(PC_SUCCESS);
}

int t2_to_t3(t2_t const *t2, t3_t *t3, double repetition_rate,
		long long time_origin) {
	long double fractpart, intpart;

	long long time;

	if ( repetition_rate == 0 || repetition_rate > 1e12 ) {
		return(PC_ERROR_ZERO_DIVISION);
	}

	t3->channel = t2->channel;

	time = t2->time - time_origin;

/* Round the pulse number to the nearest integer. */
	fractpart = modfl((long double)time*1e-12*repetition_rate, &intpart);
	t3->pulse = (long long)floor(intpart + 0.5);;
	t3->time = (long long)floor(fractpart/repetition_rate*1e12+0.5);

	return(PC_SUCCESS);
}

