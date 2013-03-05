#include "options.hpp"

#include <string>

#include "modes.hpp"

namespace po = boost::program_options;

PCOptions::PCOptions(void)
{
	this->defaults();
}

PCOptions::PCOptions(int argc, char *argv[]) 
{
	this->defaults();
	this->parse(argc, argv);
}

void PCOptions::parse(int argc, char *argv[]) 
{
	po::options_description opt("Available options");
	po::variables_map vm;

	opt.add_options()
			("version,v", 
					"Print version string.")
			("verbose,V", 
					po::value<bool>(&(this->verbose)),
					"Print debug-level information.")

			("print-every", 
					po::value<long long>(&(this->print_every)),
					"Print a status message every n entries. "
					"By default, nothing is printed.")

			("mode", 
					po::value<std::string>()->default_value(""),
					"Specify the photon mode, either t2 or t3.")
			("channels", 
					po::value<channeld_t>(&(this->channels)),
					"Specify the number of channels in the stream.")
			("order", 
					po::value<unsigned int>(&(this->order)),
					"Order of the correlation.")

			("seed", 
					po::value<unsigned int>(&(this->seed)),
					"Specify the seed for the random number generator.")

			("start", 
					po::value<dim_t>(&(this->start)),
					"The lower limit of time/pulse for the run. Only "
					"process photons which arrive after this.")
			("stop", 
					po::value<dim_t>(&(this->stop)),
					"The upper limit of time/pulse for the run. Only "
					"process photons which arrive before this.")
			("min-time-distance", "")
			("max-time-distance", "")
			("min-pulse-distance", "")
			("max-pulse-distance", "")

			("positive-only", "")
			("start-stop", "")

			("bin-width", "")
			("count-all,A", "")

			("time,x", "")
			("pulse,y", "")
			("time-scale,X", "Use a linear, log, or log-zero time scale.")
			("pulse-scale,Y", "Use a linear, log, or log-zero pulse scale.")

			("time-offsets", "")
			("pulse-ofsets", "")
			("suppress", "")
			("approximate", "")
			("exact-normalization", "")
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
	} else {
		/* Process the options. */
		std::string mode_string = vm["mode"].as<std::string>();
		if ( mode_string == "t2" ) {
			this->mode = MODE_T2;
		} else if ( mode_string == "t3" ) {
			this->mode = MODE_T3;
		} else {
			this->mode = MODE_UNKNOWN;
		}
	}
}

void
PCOptions::defaults(void)
{
	this->help = false;
	this->verbose = false;
	this->version = false;

	this->mode = MODE_UNKNOWN;

	this->channels = 2;
	this->order = 2;

	this->print_every = 0;

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
	this->set_start = false;
	this->start = 0;
	this->set_stop = false;
	this->stop = 0;

	this->time_scale = SCALE_LINEAR;
	this->pulse_scale = SCALE_LINEAR;

	this->suppress_channels = false;
	this->offset_time = false;
	this->offset_pulse = false;
}

std::ostream&
operator<<(std::ostream& out, PCOptions const& options)
{
	out << "version = " << options.version << "\n";
	out << "verbose = " << options.verbose << "\n";
	out << "help = " << options.help << "\n";

	out << "mode = " << options.mode << "\n";
	out << "channels = " << options.channels << "\n";

	out << "print_every = " << options.print_every << "\n";
	
	out << "seed = 0x" << std::hex << options.seed << "\n";
	out.setf(std::ios::dec);

	out << "queue_length = " << options.queue_length << "\n";
	out << "min_time_distance = " << options.min_time_distance << "\n";
	out << "max_time_distance = " << options.max_time_distance << "\n";
	out << "min_pulse_distance = " << options.min_pulse_distance << "\n";
	out << "max_pulse_distance = " << options.max_pulse_distance << "\n";
	out << "positive_only = " << options.positive_only << "\n";
	out << "start_stop_mode = " << options.start_stop_mode << "\n";

	out << "bin_width = " << options.bin_width << "\n";
	out << "count_all = " << options.count_all << "\n";
	out << "set_start = " << options.set_start << "\n";
	out << "start = " << options.start << "\n";
	out << "set_stop = " << options.set_stop << "\n";
	out << "stop = " << options.stop << "\n";

	out << "time_limits = " << options.time_limits << "\n";
	out << "pulse_limits = " << options.pulse_limits << "\n";
	out << "time_scale = " << options.time_scale << "\n";
	out << "pulse_scale = " << options.pulse_scale << "\n"; 

	return(out);
}

bool
PCOptions::valid(void)
{
	return( ! this->help );
}
