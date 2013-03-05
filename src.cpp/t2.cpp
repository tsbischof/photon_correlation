#include "t2.hpp"

#include <sstream>

std::ostream& 
operator<<(std::ostream& out, T2 const& t2) 
{
	out << t2.channel << "," << t2.time;

	return(out);
}

std::istream& 
operator>>(std::istream& in, T2& t2) 
{
	std::string line;
	char c1;

	std::getline(in, line);

	std::istringstream ss(line);

	ss >> t2.channel >> c1 >> t2.time;

	return(in);
}

bool 
operator<(T2 const& a, T2 const& b) 
{
	return( a.time < b.time );
}

bool 
operator>(T2 const& a, T2 const& b) 
{
	return( a.time > b.time );
}

bool 
operator==(T2 const& a, T2 const& b) 
{
	return( a.channel == b.channel and a.time == b.time );
}
