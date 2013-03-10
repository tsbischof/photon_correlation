#ifndef COUNTS_H_
#define COUNTS_H_

typedef struct {
	unsigned int bins;
	unsigned long long *counts;
} counts_t;

counts_t *counts_alloc(unsigned int const bins);
void counts_init(counts_t *counts);
int counts_increment(counts_t *counts, unsigned int const index);
int counts_increment_number(counts_t *counts, unsigned int const index,
		unsigned long long const number);
void counts_free(counts_t **counts);
int counts_nonzero(counts_t const *counts);

#endif
