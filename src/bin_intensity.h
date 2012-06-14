#ifndef BIN_INTENSITY_H_
#define BIN_INTENSITY_H_

#include <stdio.h>
#include "histogram_gn.h"
#include "t2.h"

typedef struct {
	limits_t limits;
	long long int *counts;
} bin_counts_t;

void usage(void);

#endif
