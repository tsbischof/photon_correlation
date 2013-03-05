#ifndef PHOTON_HPP_
#define PHOTON_HPP_

#include <iostream>
#include <fstream>
#include <queue>
#include <iterator>
#include <vector>

#include "types.hpp"

class Photon 
{
	public:
		friend std::ostream& operator<<(std::ostream& out, 
				Photon const& photon);
		friend std::istream& operator>>(std::istream& in, 
				Photon& photon);

		friend bool operator<(Photon const& a, Photon const& b);
		inline friend bool operator<=(Photon const& a, Photon const& b) {
				return( !operator> (a,b) ); }
		friend bool operator>(Photon const& a, Photon const& b);
		inline friend bool operator>=(Photon const& a, Photon const& b) {
				return( !operator< (a,b) ); }
		friend bool operator==(Photon const& a, Photon const& b);
		inline friend bool operator!=(Photon const& a, Photon const& b) {
				return( !operator== (a,b) ); }

		virtual dim_t window_dim(void) { return(0); }
		virtual channeld_t channel(void) { return(0); }
};

class PhotonWindow
{
	private:
	public:
}; 

class PhotonStream
{
	private:
		std::istream& in;
		PhotonWindow window;
		Photon photon;
		bool in_window;
		std::vector<bool> suppress;
	public:
		PhotonStream(void);

		bool next_photon(Photon& photon);
		PhotonWindow& next_window(void);

		bool eof(void);
		bool good(void);
		bool window_over(void);
};

#endif
