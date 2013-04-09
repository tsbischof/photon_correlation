#ifndef CONVERSIONS_H_
#define CONVERSIONS_H_

#include "t2.h"
#include "t3.h"

int t3_as_t2(t3_t const *t3, t2_t *t2);
int t3_to_t2(t3_t const *t3, t2_t *t2, double repetition_rate,
		long long time_origin);
int t2_to_t3(t2_t const *t2, t3_t *t3, double repetition_rate,
		long long time_origin);

#endif
