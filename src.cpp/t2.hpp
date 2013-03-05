#ifndef T2_HPP_
#define T2_HPP_

#include "photon.hpp"
#include "types.hpp"

class T2 : public Photon 
{
	public:	
		channeld_t channel;
		dim_t time;

		friend std::ostream& operator<<(std::ostream& out, T2 const& t2);
		friend std::istream& operator>>(std::istream& in, T2& t2);  

		dim_t window_dim(void) { return(this->time); }
};

#endif
