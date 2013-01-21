#include "options.hpp"

namespace po = boost::program_options;

Options::Options(int argc, char *argv[]) 
{
	po::options_description generic("Generic options");
	generic.add_options()
			("version,v", "Print version string.")
			("verbose,V", "Print debug-level information.")
			("help,h", "Print a help message.")
			("time-scale", "Use a linear, log, or log-zero (lowest bin has "
					"zero for a lower limit) scale for the time axis.");
}

std::ostream&
operator<<(std::ostream& out, Options const& options)
{
	out << "version = " << options.version << "\n";
	out << "verbose = " << options.verbose << "\n";
	out << "help = " << options.help << "\n";

	out << "filename_in = " << options.filename_in << "\n";
	out << "filename_out = " << options.filename_out << "\n";

	out << "mode = " << options.mode << " (" 
			<< options.mode_string << ")" << "\n";
	out << "channels = " << options.channels << "\n";

	out << "print_every = " << options.print_every << "\n";
	
	out << "binary_in = " << options.binary_in << "\n";
	out << "binary_out = " << options.binary_out << "\n";

	out << "seed = 0x" << std::hex << options.seed << "\n";

	out << "queue_length = " << options.queue_length << "\n";
	out << "min_time_distance = " << options.min_time_distance << "\n";
	out << "max_time_distance = " << options.max_time_distance << "\n";
	out << "min_pulse_distance = " << options.min_pulse_distance << "\n";
	out << "max_pulse_distance = " << options.max_pulse_distance << "\n";
	out << "positive_only = " << options.positive_only << "\n";
	out << "start_stop_mode = " << options.start_stop_mode << "\n";

	out << "bin_width = " << options.bin_width << "\n";
	out << "count_all = " << options.count_all << "\n";
	out << "set_start_time = " << options.set_start_time << "\n";
	out << "start_time = " << options.start_time << "\n";
	out << "set_stop_time = " << options.set_stop_time << "\n";
	out << "stop_time = " << options.stop_time << "\n";

	out << "time_limits = " << options.time_limits << "\n";
	out << "pulse_limits = " << options.pulse_limits << "\n";
	out << "time_scale = " << options.time_scale << "\n";
	out << "pulse_scale = " << options.pulse_scale << "\n";

	return(out);
}
