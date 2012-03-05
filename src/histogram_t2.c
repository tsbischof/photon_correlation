#include <stdio.h>

#include "histogram.h"
#include "histogram_t2.h"
#include "t2.h"
#include "error.h"
#include "combinations.h"

t2_histograms_t *allocate_t2_histograms(options_t *options) {
	int result = 0;
	int i;
	int j;
	combination_t *combination;
	t2_histograms_t *histograms = NULL;

	histograms = (t2_histograms_t *)malloc(sizeof(t2_histograms_t));
	combination = allocate_combination(options->channels, options->order);

	if ( histograms != NULL && combination != NULL ) {
		histograms->n_histograms = n_combinations(options->channels,
				options->order);
		histograms->time_limits = options->time_limits;
		histograms->bins_per_histogram = pow_int(options->time_limits.bins,
				options->order-1);

		histograms->histograms = (t2_histogram_t *)malloc(
				sizeof(t2_histogram_t)*histograms->n_histograms);

		if ( histograms->histograms == NULL ) {
			free_t2_histograms(&histograms);
		} else {
			for ( i = 0; i < histograms->n_histograms; i++ ) {
				histograms->histograms[i].counts = (unsigned int *)malloc(
						sizeof(unsigned int)*histograms->bins_per_histogram);

				if ( histograms->histograms[i].counts == NULL ) {
					result = -1;
					i = histograms->n_histograms;
				} else {
					for ( j = 0; j < histograms->bins_per_histogram; j++ ) {
						histograms->histograms[i].counts[j] = 0;
					}
				} 

				histograms->histograms[i].channels = (unsigned int *)malloc(
						sizeof(unsigned int)*histograms->order);
				if ( histograms->histograms[i].channels == NULL ) {
					result = - 1;
					i = histograms->n_histograms;
				} else {
					for ( j = 0; j < histograms->order; j++) { 
						histograms->histograms[i].channels[j] = 
								combination->digits[j];
					}
				}

				next_combination(combination);
			}
		}
	} else {
		result = -1;
	}

	if ( result ) {
		error("Could not allocate memory for histograms.\n");
		free_t2_histograms(&histograms);
	}

	free_combination(&combination);

	return(histograms);
}

void free_t2_histograms(t2_histograms_t **histograms) {
	int i;
	if ( *histograms != NULL ) {
		if ( (*histograms)->histograms != NULL ) {
			for ( i = 0; i < (*histograms)->n_histograms; i++ ) {
				free((*histograms)->histograms[i].counts);
				free((*histograms)->histograms[i].channels);
			}
			free((*histograms)->histograms);
		}
		free(*histograms);
	}
}

void print_t2_histograms(FILE *out_stream, t2_histograms_t *histograms) {
	int i;

	for ( i = 0; i < histograms->n_histograms; i++ ) {
		/* Looping over the histograms. */
	}
}

t2_correlated_t *allocate_t2_correlated(options_t *options) {
	t2_correlated_t *record = NULL;

	record = (t2_correlated_t *)malloc(sizeof(t2_correlated_t));
	if ( record != NULL ) { 
		record->records = (t2_t *)malloc(sizeof(t2_t)*(options->order-1));
		if ( record->records == NULL ) {
			free_t2_correlated(&record);
		}
	}

	return(record);
}

void free_t2_correlated(t2_correlated_t **record) {
	if ( *record != NULL ) {
		if ( (*record)->records != NULL ) {
			free((*record)->records);
		}
		free(*record);
	}
}

int next_t2_correlated(FILE *in_stream, t2_correlated_t *record,
		 options_t *options) {
	int result;
	int i;

	result = (fscanf(in_stream, "%u", &(*record).ref_channel) != 1);

	if ( result && !feof(in_stream) ) {
		error("Could not read reference channel from stream.\n");
	} else {
		for ( i = 0; i < options->order - 1; i++ ) {
			result = ( fscanf(in_stream, ",%u,%lld", 
					&(*record).records[i].channel,
					&(*record).records[i].time) != 2);
			if ( result && !feof(in_stream)) {
				error("Could not read correlated record (index %d).\n", i);
				i = options->order;
			}
		}

	}
			
	return(result);
}

int histogram_t2(FILE *in_stream, FILE *out_stream, options_t *options) {
	t2_correlated_t *record;
	t2_histograms_t *histograms;
	combinations_t *combinations;
	int result = 0;
	int i;

	if ( options->order == 1 ) {
		error("Correlation of order 1 is not implemented for t2 mode.\n");
		return(-1);
	}

	histograms = allocate_t2_histograms(options);
	record = allocate_t2_correlated(options);
	combinations = make_combinations(options->channels, options->order);
	
	if ( record == NULL || combinations == NULL ) {
		error("Could not allocate memory for the histogram run.\n");
		result = -1;
	} else {
		while ( ! next_t2_correlated(in_stream, record, options) ) {
			fprintf(out_stream, "%u", record->ref_channel);
			for ( i = 0; i < options->order-1; i++ ) {
				fprintf(out_stream, ",%u,%lld", 
						record->records[i].channel,
						record->records[i].time);
			}
			fprintf(out_stream, "\n");
		}

		print_t2_histograms(out_stream, histograms);
	}

	free_t2_histograms(&histograms);
	free_combinations(&combinations);
	free_t2_correlated(&record);
	
	return(0);
}
