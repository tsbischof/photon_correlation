#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_

#include <string>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>
namespace po = boost::program_options;

#include "types.hpp"
#include "limits.hpp"

/* to do: modify this to provide subsets of options for each program */
class PCOptions
{
	public:
		PCOptions(void);
		PCOptions(int argc, char *argv[]);

		void parse(int argc, char *argv[]);
		friend std::istream& operator>>(std::istream& in, PCOptions& options);
		friend std::ostream& operator<<(std::ostream& out, 
				PCOptions const& options);

		void defaults(void);
		bool valid(void);

// General
		bool help;
		bool version;
		bool verbose;

		unsigned int mode;

		channeld_t channels;
		unsigned int order;

		long long print_every;

		unsigned int seed;

// Correlate
		size_t queue_length;
		dim_t min_time_distance;
		dim_t max_time_distance;
		dim_t min_pulse_distance;
		dim_t max_pulse_distance;
		bool positive_only;
		bool start_stop_mode;

// Intensity
		dim_t bin_width;
		bool count_all;
		bool set_start;
		dim_t start;
		bool set_stop;
		dim_t stop;

// Histogram
		Limits time_limits;
		Limits pulse_limits;
		unsigned int time_scale;
		unsigned int pulse_scale;

// Channels
		bool suppress_channels;
		std::vector<bool> suppressed_channels;
		bool offset_time;
		std::vector<int64_t> time_offsets;
		bool offset_pulse;
		std::vector<int64_t> pulse_offsets;
};

#endif
