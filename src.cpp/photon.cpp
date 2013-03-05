#include "photon.hpp"

std::ostream& 
operator<<(std::ostream& out, Photon const& photon)
{
	out << "Overload me!\n";
	return(out);
}

std::istream& 
operator>>(std::istream& in, Photon& photon)
{
	return(in);
}

bool 
operator<(Photon const& a, Photon const& b);
bool 
operator>(Photon const& a, Photon const& b);
bool 
operator==(Photon const& a, Photon const& b);

bool
PhotonStream::next_photon(Photon& photon)
{
	return( this->in >> this->photon );
}
