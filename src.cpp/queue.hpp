#ifndef QUEUE_HPP_
#define QUEUE_HPP_

template <class T>
class Queue
{
	private:
		T data[];
	public:
		size_T size(void);
		bool empty(void);

		operator[];
		at();
		front(void);
		back(void);
		data(void);

		fill();
		swap();

		sort();
}

#endif
