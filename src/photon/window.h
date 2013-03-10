#ifndef WINDOW_H_
#define WINDOW_H_

#include "photon.h"

typedef struct {
	long long lower;
	long long upper;

	long long width;
	int set_lower_bound;
	int set_upper_bound;
	long long lower_bound;
	long long upper_bound;
} photon_window_t;

void photon_window_init(photon_window_t *window, 
		long long const bin_width,
		int const set_lower_bound, long long const lower_bound,
		int const set_upper_bound, long long const upper_bound);
int photon_window_next(photon_window_t *window);
int photon_window_contains(photon_window_t const *window, 
		long long const value);

#endif
