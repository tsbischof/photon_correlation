#include "window.h"
#include "../error.h"

void photon_window_init(photon_window_t *window, 
		long long const bin_width,
		int const set_lower_bound, long long const lower_bound,
		int const set_upper_bound, long long const upper_bound) {
	window->width = bin_width;

	if ( window->width == 0 ) {
		window->set_lower_bound = false;
		window->lower_bound = 0;
		window->set_upper_bound = false;
		window->upper_bound = 0;
	} else {
		window->set_lower_bound = set_lower_bound;
		window->set_upper_bound = set_upper_bound;
		window->lower_bound = lower_bound;
		window->upper_bound = upper_bound;
	
		if ( set_lower_bound ) {
			window->lower = lower_bound;
		} else {
			window->lower = 0;
		}
	
		window->upper = window->lower + window->width;
	
		if ( set_upper_bound && window->upper > window->upper_bound ) {
			window->upper = window->upper_bound;
		}
	}
}

int photon_window_next(photon_window_t *window) {
/* Check that the limits have not been played with by intensity or otehr
 * programs */
	window->upper += window->width;
	window->lower = window->upper - window->width;

	if ( window->set_upper_bound && 
			window->upper > window->upper_bound ) {
		if ( window->lower < window->upper_bound ) {
			window->upper = window->upper_bound;
			/* Still within the bound, just trim it down a bit. */
			return(PC_SUCCESS);
		} else {
			/* Outside the upper bound. */
			return(PC_WINDOW_EXCEEDED);
		}
	} else {
		return(PC_SUCCESS);
	}
}

int photon_window_contains(photon_window_t const *window, 
		long long const value) {
	if ( window->set_lower_bound || window->set_upper_bound ) {
		if ( window->set_lower_bound && value < window->lower) {
			return(PC_RECORD_BEFORE_WINDOW);
		} else if ( window->set_upper_bound && window->upper <= value ) {
			return(PC_RECORD_AFTER_WINDOW);
		} else {
			return(PC_RECORD_IN_WINDOW);
		}
	} else {
		if ( value < window->lower ) {
			return(PC_RECORD_BEFORE_WINDOW);
		} else if ( window->upper <= value ) {
			return(PC_RECORD_AFTER_WINDOW);
		} else {
			return(PC_RECORD_IN_WINDOW);
		}
	}
}
