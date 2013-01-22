#ifndef PHOTON_HPP_
#define PHOTON_HPP_

#include <iostream>
#include <queue>
#include <iterator>

class Photon 
{
	public:
		friend std::ostream& operator<<(std::ostream& out, 
				Photon const& photon);
		friend std::istream& operator>>(std::istream& in, 
				Photon& photon);
		int fread(std::fstream& in);
		int fwrite(std::fstream& out);

		friend bool operator<(Photon const& a, Photon const& b);
		inline friend bool operator<=(Photon const& a, Photon const& b) {
				return( !operator> (a,b) ); }
		friend bool operator>(Photon const& a, Photon const& b);
		inline friend bool operator>=(Photon const& a, Photon const& b) {
				return( !operator< (a,b) ); }
		friend bool operator==(Photon const& a, Photon const& b);
		inline friend bool operator!=(Photon const& a, Photon const& b) {
				return( !operator== (a,b) ); }
};

template <class PhotonT>
class Window : std::iterator<output_iterator_tag, PhotonT>
{
	private:
		PhotonT lower;
		PhotonT upper;
		int64_t width;
	public:
		Window(PhotonT const& lower, int64_t const width);
		Window(Window const& mit);

		Window& operator++();
		Window& operator*();

		friend std::ostream& operator<<(std::ostream& out, 
				Window const& window);
		friend std::istream& operator>>(std::istream& in,
				Window& window);

		bool contains(PhotonT const& value);
		bool ahead_of(PhotonT const& value);
};

/* The PhotonStream class has a number of tasks to perform:
 * 1. Read data from an incoming stream.
 * 2. Decode the data, conditioning as necessary:
 *    a. Dropping photons on particular channels
 *    b. Adding time/pulse offsets
 * 3. Yield photons in the current window, or indicate that the window must
 *    be advanced.
 */
class PhotonStream
{
/*	private:
		queue();
		compare()
		mode()
	public:
		PhotonQueue();

		operator==();
		inline friend operator!=() { return( ! operator== ()) };
		operator++();
		

		front();
		back();
		push();
		pop();
		operator[];

		sort();
		next_window();
		window(); */
};

#endif
