#ifndef T2_HPP_
#define T2_HPP_

#include "photon.hpp"

class T2 : public Photon 
{
	public:	
		uint32_t channel;
		int64_t time;
		friend std::ostream& operator<<(std::ostream& out, T2 const& t2);
		friend std::istream& operator>>(std::istream& in, T2& t2);  
};

class T2Stream : public PhotonStream
{

};


#endif
