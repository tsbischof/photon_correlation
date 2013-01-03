#include "photon.hpp"

std::ostream& operator<<(std::ostream& out, Photon const& photon) {
	std::cerr << "Overload me!" << std::endl;
	return(out);
}

std::istream& operator>>(std::istream& in, Photon& photon) {
	std::cerr << "Overload me!" << std::endl;
	return(in);
}

bool operator<(Photon const& a, Photon const& b);
bool operator>(Photon const& a, Photon const& b);
bool operator==(Photon const& a, Photon const& b);
