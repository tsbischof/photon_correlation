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

int limits_parse(const char *str, limits_t *limits);
int scale_parse(const char *str, int *scale);

#endif
