#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_

#include <string>
#include <iostream>
#include <vector>

#include "limits.hpp"

class Options
{
	public:
		bool help;
		bool version;
		bool verbose;

		std::string filename_in;
		std::string filename_out;

		std::string mode_string;
		int mode;

		unsigned int channels;
		unsigned int order;

		unsigned int print_every;

		bool binary_in;
		bool binary_out;

		unsigned int seed;

// Correlate
		size_t queue_length;
		uint64_t min_time_distance;
		uint64_t max_time_distance;
		uint64_t min_pulse_distance;
		uint64_t max_pulse_distance;
		bool positive_only;
		bool start_stop_mode;

// Intensity
		uint64_t bin_width;
		bool count_all;
		bool set_start_time;
		int64_t start_time;
		bool set_stop_time;
		int64_t stop_time;

// Histogram
		Limits time_limits;
		Limits pulse_limits;
		int time_scale;
		int pulse_scale;

// Channels
		bool suppress_channels;
		std::vector<bool> suppressed_channels;
		bool offset_time;
		std::vector<int64_t> time_offsets;
		bool offset_pulse;
		std::vector<int64_t> pulse_offsets;
		
		Options(int argc, char *argv[]);
		friend std::istream& operator>>(std::istream& in, Options& options);
		friend std::ostream& operator<<(std::ostream& out, 
				Options const& options);

		void default_options(void);
};

#endif
