#include "t2.hpp"

#include <sstream>
#include <iostream>

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

	std::getline(in, line,',');
	std::istringstream(line) >> t2.channel;
	std::getline(in, line);
	std::istringstream(line) >> t2.time; 

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
	return( a.channel == b.channel && a.time == b.time );
}
