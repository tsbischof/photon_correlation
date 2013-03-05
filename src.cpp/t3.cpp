#include "t3.hpp"

#include <sstream>

std::ostream& 
operator<<(std::ostream& out, const T3& t3) 
{
	out << t3.channel << "," << t3.pulse << "," << t3.time;

	return(out);
}

std::istream& 
operator>>(std::istream& in, T3& t3) 
{
	std::string line;
	char c1, c2;

	std::getline(in, line);
	std::istringstream ss(line);

	ss >> t3.channel >> c1 >> t3.pulse >> c2 >> t3.time;

	return(in);
}

bool 
operator<(T3 const& a, T3 const& b) 
{
	if ( a.pulse < b.pulse ) {
		return(true);
	} else if ( a.pulse == b.pulse ) {
		return(a.time < b.time);
	} else {
		return(false);
	}
}

bool 
operator>(T3 const& a, T3 const& b) 
{
	if ( a.pulse > b.pulse ) {
		return(true);
	} else if ( a.pulse == b.pulse ) {
		return(a.time > b.time);
	} else {
		return(false);
	}
}

bool 
operator==(T3 const& a, T3 const& b) 
{
	return( a.channel == b.channel and
			a.pulse == b.pulse and
			a.time == b.time );
}
