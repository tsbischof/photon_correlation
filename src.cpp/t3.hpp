#ifndef T3_HPP_
#define T3_HPP_

#include "photon.hpp"

class T3 : public Photon 
{
	public:
		uint32_t channel;
		int64_t pulse;
		int32_t time;
};

#endif
