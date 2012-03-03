#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#define SCALE_UNKNOWN -1
#define SCALE_LINEAR 1
#define SCALE_LOG 2

typedef struct {
	long long int lower;
	int bins;
	long long int upper;
} limits_t;

typedef struct {
	char *in_filename;
	char *out_filename;
	char *mode_string;
	int mode;
	char *time_string;
	limits_t time_limits;
	char *pulse_string;
	limits_t pulse_limits;
	int channels;
	int order;
	char *time_scale_string;
	int time_scale;
	char *pulse_scale_string;
	int pulse_scale;
} options_t;

void usage(void);
int str_to_limits(char *str, limits_t *limits);
int scale_parse(char *str, int *scale);

#endif
