#ifndef LIMITS_HPP_
#define LIMITS_HPP_

#define SCALE_LINEAR          0
#define SCALE_LOG             1
#define SCALE_LOG_ZERO        2

#include <iostream>
#include <vector>
#include <queue>
#include <iterator>

template <class T>
class Window
{
	private:
		T lower;
		T upper;
		int64_t width;
	public:
		Window(T const& lower, int64_t const width);
		Window(Window const& mit);

		Window& operator++();
		Window& operator*();

		friend std::ostream& operator<<(std::ostream& out, 
				Window const& window);
		friend std::istream& operator>>(std::istream& in,
				Window& window);

		bool contains(T const& value);
		bool ahead_of(T const& value);
};

class Limits
{
	public:
		double lower;
		unsigned int n_bins;
		double upper;

		Limits(void);
		Limits(double lower, unsigned int n_bins, double upper);

		friend std::ostream& operator<<(std::ostream& out, 
				Limits const& limits);
		friend std::istream& operator>>(std::istream& in, Limits& limits);
};

class Edges
{
	private:
		int scale;
		Limits limits;
		std::vector<Window<double>> bounds;
	public:
		Edges(Limits& limits);

		friend std::ostream& operator<<(std::ostream& out, Edges const& edges);
		friend std::istream& operator>>(std::istream& in, Edges& edges);

		friend Window<double>& operator[](int const index);
		int index(double value);
}

#endif
