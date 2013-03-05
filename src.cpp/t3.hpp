#ifndef T3_HPP_
#define T3_HPP_

#include "photon.hpp"

class T3 : public Photon 
{
	public:
		channeld_t channel;
		dim_t pulse;
		dim_t time;

		friend std::ostream& operator<<(std::ostream& out, T3 const& t3);
		friend std::istream& operator>>(std::istream& in, T3& t3);  

		dim_t window_dim(void) { return(this->pulse); }
};

#endif
