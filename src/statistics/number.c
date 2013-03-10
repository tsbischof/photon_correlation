#include "number.h"
#include "../photon/stream.h"
#include "../modes.h"

#include "../error.h"

photon_number_t *photon_number_alloc(unsigned int max_number) {
	photon_number_t *number = NULL;

	number = (photon_number_t *)malloc(sizeof(photon_number_t));

	if ( number == NULL ) {
		return(number);
	} 

	number->max_number = max_number;
	number->counts = counts_alloc(number->max_number+1);

	if ( number->counts == NULL ) {
		photon_number_free(&number);
		return(number);
	}

	return(number);
}

void photon_number_init(photon_number_t *number) {
	number->first_seen = false;
	number->last_pulse = 0;
	number->current_seen = 0;
	number->max_seen = 0;
	
	counts_init(number->counts);
}

void photon_number_free(photon_number_t **number) {
	if ( number != NULL ) {
		counts_free(&((*number)->counts));
		free(*number);
	}
}

int photon_number_push(photon_number_t *number, t3_t const *t3) {
	int result = PC_SUCCESS;

	if ( ! number->first_seen ) {
		number->first_seen = true;
		number->last_pulse = t3->pulse;
		number->current_seen++;

		return(photon_number_check_max(number));
	} else {
		if ( t3->pulse == number->last_pulse ) {
			number->current_seen++;
			return(photon_number_check_max(number));
		} else {
			result = photon_number_increment(number, number->current_seen, 1);
			photon_number_increment(number, 
					0, 
					t3->pulse - number->last_pulse - 1);

			number->current_seen = 1;
			number->last_pulse = t3->pulse;

			if ( result == PC_SUCCESS ) {
				return(photon_number_check_max(number));
			} else {
				return(result);
			}
		}
	}
}

int photon_number_increment(photon_number_t *number, 
		unsigned int const n_photons, unsigned long long seen) {
	int result = counts_increment_number(number->counts, n_photons, seen);

	if ( result != PC_SUCCESS ) {
		error("Could not increment for %u photons.\n", n_photons);
	} 

	return(result);
}
	
int photon_number_check_max(photon_number_t *number) {
	if ( number->current_seen > number->max_seen ) {
		number->max_seen = number->current_seen;
	}

	if ( number->max_seen <= number->max_number ) {
		return(PC_SUCCESS);
	} else {
		error("Too many photons: %u\n", number->max_seen);
		return(PC_ERROR_INDEX);
	}
}

int photon_number_flush(photon_number_t *number) {
	return(photon_number_increment(number, number->current_seen, 1));
}

int photon_number_fprintf(FILE *stream_out, photon_number_t const *number) {
	int i;

	for ( i = 0 ; i < number->max_seen+1; i++ ) {
		if ( number->counts->counts[i] != 0 ) {
			fprintf(stream_out, "%d,%llu\n", i, number->counts->counts[i]);
		}
	}

	return( ferror(stream_out) ? PC_ERROR_IO : PC_SUCCESS );
}

int photon_number(FILE *stream_in, FILE *stream_out, 
		pc_options_t const *options) { 
	int result = PC_SUCCESS;
	photon_stream_t *photons;
	photon_number_t *number;

	number = photon_number_alloc(options->channels * 64);
	photons = photon_stream_alloc(MODE_T3);

	if ( number == NULL || photons == NULL ) {
		error("Could not allocate photon stream or numbers.\n");
		result = PC_ERROR_MEM;
	}

	photon_number_init(number);
	photon_stream_init(photons, stream_in);
	photon_stream_set_unwindowed(photons);

	debug("Max photons per pulse: %u\n", number->max_number);

	if ( result == PC_SUCCESS) {
		while ( photon_stream_next_photon(photons) == PC_SUCCESS ) {
			photon_number_push(number, (t3_t *)photons->photon);
		}

		photon_number_flush(number);
		photon_number_fprintf(stream_out, number);
	}

	photon_stream_free(&photons);
	photon_number_free(&number);

	return(result);
}
