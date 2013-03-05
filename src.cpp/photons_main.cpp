#include "options.hpp"
#include "modes.hpp"
#include "photon.hpp"
#include "t2.hpp"
#include "t3.hpp"
#include "echo.hpp"

int main(int argc, char *argv[]) {
	PCOptions options(argc, argv);
	std::istream &in = std::cin;
	std::ostream &out = std::cout;
	T2 t2;
	T3 t3;

	if ( options.valid() ) {
		if ( options.mode == MODE_T2 ) {
			echo(in, out, t2);
		} else if ( options.mode == MODE_T3 ) {
			echo(in, out, t3);
		} else {
			std::cerr << "Mode not recognized: " << options.mode << "\n";
		}
	}

	return(0);
}
