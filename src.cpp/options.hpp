#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_

#include <string>
#include <iostream>

#include <boost/program_options.hpp>

#include "histogram.hpp"
#include "modes.hpp"

class Options
{
	public:
		bool help = false;
		bool version = false;
		bool verbose = false;

		std::string filename_in = "";
		std::string filename_out = "";

		std::string mode_string = "";
		int mode = MODE_UNKNOWN;

		unsigned int channels = 2;
		unsigned int order = 2;

		unsigned int print_every = 0;

		bool binary_in = false;
		bool binary_out = false;

		unsigned int seed = 0xDEADBEEF;

// Correlate
		size_t queue_length = 1024*1024;
		uint64_t min_time_distance;
		uint64_t max_time_distance;
		uint64_t min_pulse_distance;
		uint64_t max_pulse_distance;
		bool positive_only = false;
		bool start_stop_mode = false;

// Intensity
		uint64_t bin_width;
		bool count_all = false;
		bool set_start_time = false;
		int64_t start_time;
		bool set_stop_time = false;
		int64_t stop_time;

// Histogram
		Limits time_limits;
		Limits pulse_limits;
		int time_scale = SCALE_LINEAR;
		int pulse_scale = SCALE_LINEAR;

// Channels
		bool suppress_channels = false;
		std::vector<bool> suppressed_channels;
		bool offset_time = false;
		std::vector<int64_t> time_offsets;
		bool offset_pulse = false;
		std::vector<int64_t> pulse_offsets;
		
		Options(int argc, char *argv[]);
		friend std::istream& operator>>(std::istream& in, Options& options);
		friend std::ostream& operator<<(std::ostream& out, 
				Options const& options);
};

#endif
