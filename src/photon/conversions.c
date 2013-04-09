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
	long long time;
	int64_t repetition_time;

	if ( repetition_rate == 0 || repetition_rate > 1e12 ) {
		return(PC_ERROR_ZERO_DIVISION);
	}

	repetition_time = 1e12 / repetition_rate;

/* The behavior we want here is a floor instead of truncation: directly invert
 * the modulo arithmetic
 */
	t3->channel = t2->channel;

	time = t2->time - time_origin;
	if ( time < 0 ) {
		t3->pulse = -(-time / repetition_time) - 1;
	} else {
		t3->pulse = time / repetition_time;
	}

	t3->time = time - (t3->pulse * repetition_time);

	return(PC_SUCCESS);
}

