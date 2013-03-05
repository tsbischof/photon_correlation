#ifndef ECHO_HPP_
#define ECHO_HPP_

#include <fstream>

template <class T>
void
echo(std::istream& in, std::ostream& out, T& t)
{
	while ( in.good() ) {
		if ( in >> t ) {
			out << t << "\n";
		}
	}
}

#endif
