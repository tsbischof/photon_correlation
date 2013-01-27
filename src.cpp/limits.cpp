#include "limits.hpp"

#include <sstream>

Limits::Limits(void)
{
	this->lower = 0;
	this->upper = 0;
	this->n_bins = 0;
}

Limits::Limits(double lower, unsigned int n_bins, double upper)
{
	this->lower = lower;
	this->n_bins = n_bins;
	this->upper = upper;
}

std::ostream&
operator<<(std::ostream& out, Limits const& limits)
{
	out << limits.lower << "," << limits.n_bins << "," << limits.upper;

	return(out);
}

std::istream&
operator>>(std::istream& in, Limits& limits)
{
	std::string line;

	std::getline(in, line, ',');
	std::istringstream(line) >> limits.lower;
	std::getline(in, line, ',');
	std::istringstream(line) >> limits.n_bins;
	std::getline(in, line);
	std::istringstream(line) >> limits.upper;

	return(in);
}


