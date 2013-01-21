#ifndef HISTOGRAM_HPP_
#define HISTOGRAM_HPP_

#define SCALE_LINEAR          0
#define SCALE_LOG             1
#define SCALE_LOG_ZERO        2

struct Limits
{
	int64_t lower;
	size_t n_bins;
	int64_t upper;
};

#endif
