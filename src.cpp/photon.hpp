#ifndef PHOTON_HPP_
#define PHOTON_HPP_

#include <iostream>
#include <queue>

class Photon 
{
	public:
		friend std::ostream& operator<<(std::ostream& out, 
				Photon const& photon);
		friend std::istream& operator>>(std::istream& in, 
				Photon& photon);
		int fread(std::fstream& in);
		int fwrite(std::fstream& out);

		friend bool operator<(Photon const& a, Photon const& b);
		inline friend bool operator<=(Photon const& a, Photon const& b) {
				return( !operator> (a,b) ); }
		friend bool operator>(Photon const& a, Photon const& b);
		inline friend bool operator>=(Photon const& a, Photon const& b) {
				return( !operator< (a,b) ); }
		friend bool operator==(Photon const& a, Photon const& b);
		inline friend bool operator!=(Photon const& a, Photon const& b) {
				return( !operator== (a,b) ); }
};

template <class T>
class PhotonQueue: public std::queue
{
};

#endif
