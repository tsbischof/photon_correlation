#include <fstream>

#include "photon.hpp"
#include "t2.hpp"

int main(int argc, char *argv[]) {
	std::ifstream stream_in;
	std::ostream& stream_out = std::cout;
	T2 t2;

	stream_in.open("t2.txt");

	while ( stream_in.good() ) {
		if ( stream_in >> t2 ) {
			stream_out << t2 << '\n';
		}

	} 

	stream_in.close();

	return(0);
}
