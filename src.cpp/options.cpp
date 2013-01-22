#include "options.hpp"

#include <string>

#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>

#include "modes.hpp"

namespace po = boost::program_options;

Options::Options(int argc, char *argv[]) 
{
	po::options_description opt("Available options");
	po::variables_map vm;

	this->default_options();

	opt.add_options()
			("version,v", "Print version string.")
			("verbose,V", "Print debug-level information.")
			("help,h", "Print a help message.")

			("print-every,p", "Print a status message every n entries. "
					"By default, nothing is printed.")


			("file-in,i", 
					po::value<std::string>(&(this->filename_in))->default_value(""),
					"Specify the input filename. By default, "
					"this is stdin.")
			("file-out,o", "Specify the output filename. By defalut, "
					"this is stdout.")
			
			("mode,m", "Specify the photon mode, either t2 or t3.")
			("channels,c", "Specify the number of channels in the stream.")
			("order,g", "Order of the correlation.")

			("seed,K", "Specify the seed for the random number generator.")

			("queue-size,q", "Specify the size of the fixed photon queue. "
					"This must be large enough to handle the scale of the "
					"correlation, but not too large to waste memory.")

			("start,f", "The lower limit of time/pulse for the run. Only "
					"process photons which arrive after this.")
			("stop,F", "The upper limit of time/uplse for the run. Only "
					"process photons which arrive before this.")
			("min-time-distance,d", "")
			("max-time-distance,D", "")
			("min-pulse-distance,e", "")
			("max-pulse-distance,E", "")

			("positive-only,P", "")
			("start-stop,S", "")

			("bin-width,w", "")
			("count-all,A", "")

			("time,x", "")
			("pulse,y", "")
			("time-scale,X", "Use a linear, log, or log-zero time scale.")
			("pulse-scale,Y", "Use a linear, log, or log-zero pulse scale.")

			("time-offsets,u", "")
			("pulse-ofsets,U", "")
			("suppress,s", "")
			("approximate,B", "")
			("exact-normalization,Z", "")
			;
			

	try {
		po::store(po::parse_command_line(argc, argv, opt), vm);
		po::notify(vm);
	} catch (po::unknown_option e) {
		this->help = true;
	}

	if ( vm.count("help") ) {
		this->help = true;
	}

	if ( this->help ) {
		std::cerr << opt << "\n";
	}
}

void
Options::default_options(void)
{
	this->help = false;
	this->verbose = false;
	this->version = false;

	this->filename_in = "";
	this->filename_out = "";

	this->mode_string = "";
	this->mode = MODE_UNKNOWN;

	this->channels = 2;
	this->order = 2;

	this->print_every = 0;

	this->binary_in = false;
	this->binary_out = false;

	this->seed = 0xdeadbeef;

	this->queue_length = 1024*1024;
	this->min_time_distance = 0;
	this->max_time_distance = 0;
	this->min_pulse_distance = 0;
	this->max_pulse_distance = 0;
	this->positive_only = false;
	this->start_stop_mode = false;

	this->bin_width = 1000000000;
	this->count_all = false;
	this->set_start_time = false;
	this->start_time = 0;
	this->set_stop_time = false;
	this->stop_time = 0;

	this->time_scale = SCALE_LINEAR;
	this->pulse_scale = SCALE_LINEAR;

	this->suppress_channels = false;
	this->offset_time = false;
	this->offset_pulse = false;
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
