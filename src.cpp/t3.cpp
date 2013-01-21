#include "t3.hpp"

#include <sstream>
#include <iostream>

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

	std::getline(in, line, ',');
	std::istringstream(line) >> t3.channel;
	std::getline(in, line, ',');
	std::istringstream(line) >> t3.pulse;
	std::getline(in, line);
	std::istringstream(line) >> t3.time;

	return(in);
}
