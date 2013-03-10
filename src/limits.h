#ifndef LIMITS_H_
#define LIMITS_H_

#define SCALE_UNKNOWN -1
#define SCALE_LINEAR 1
#define SCALE_LOG 2
#define SCALE_LOG_ZERO 3

typedef struct {
	double lower;
	size_t bins;
	double upper;
} limits_t;

int limits_parse(limits_t *limits, const char *str);
int limits_valid(limits_t const *limits);

int scale_parse(int *scale, const char *str);

#endif
